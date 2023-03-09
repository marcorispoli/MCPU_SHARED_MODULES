#include "device_command_interface.h"
#include <QStringConverter>

uchar SocketItem::idcount = 0;

deviceCommandInterface::deviceCommandInterface(QString ipaddress, int port):QTcpServer()
{
    socketList.clear();
    localip = QHostAddress(ipaddress);
    localport = port;

}

deviceCommandInterface::~deviceCommandInterface()
{
    if(socketList.size()){
        for(int i = 0; i<socketList.size(); i++ ){
            if(socketList[i]->socket != nullptr) {
                socketList[i]->socket->close();
                socketList[i]->socket->deleteLater();
            }
        }
    }
}

bool deviceCommandInterface::CommandStart(void)
{
    QString stringa = QString("DEVICE COMMAND INTERFACE STARTED AT: PORT:%2 ").arg(localport);
    qDebug() << stringa;

    if (!this->listen(localip,localport)) return false;
    return true;

}


void deviceCommandInterface::incomingConnection(qintptr socketDescriptor)
{

    SocketItem* item = new SocketItem();

    item->socket = new QTcpSocket(this); // Create a new socket
    if(!item->socket->setSocketDescriptor(socketDescriptor))
    {
        delete item->socket;
        delete item;
        return;
    }

    item->socket->setSocketOption(QAbstractSocket::LowDelayOption,1);
    socketList.append(item);


    // Interface signal connection
    connect(item->socket,SIGNAL(readyRead()), item, SLOT(socketRxData()),Qt::UniqueConnection);
    connect(item->socket,SIGNAL(disconnected()),item, SLOT(disconnected()),Qt::UniqueConnection);


    connect(item,SIGNAL(itemDisconnected(ushort )),this, SLOT(disconnected(ushort )),Qt::UniqueConnection);
    connect(item->socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),item,SLOT(socketError(QAbstractSocket::SocketError)),Qt::UniqueConnection);    
    connect(item,SIGNAL(execCommandSgn(uchar , QList<QByteArray> )),this, SLOT(execCommandSlot(uchar , QList<QByteArray> )),Qt::UniqueConnection);

    // From the Device to the Every connected Client of the command interface
    connect(this,SIGNAL(sendToClientSgn(uchar , QList<QByteArray> )),item, SLOT(socketTxData(uchar , QList<QByteArray> )),Qt::QueuedConnection);

    return;
 }


void SocketItem::disconnected(void){
    emit itemDisconnected(this->id);
}

void SocketItem::socketError(QAbstractSocket::SocketError error)
{
    emit itemDisconnected(this->id);
    return;
}


void deviceCommandInterface::disconnected(ushort id)
{

    for(int i =0; i < socketList.size(); i++ ){
        if(socketList[i]->id == id){

            disconnect(socketList[i]);
            socketList[i]->socket->deleteLater();
            delete socketList[i];
            socketList.remove(i);
            return;
        }
    }

}

void deviceCommandInterface::execCommandSlot(uchar socket_id, QList<QByteArray> command){
    funcPtr function = commandList.value(command.at(0));

    if(function) (*function)(this, socket_id, &command);
    else  qDebug() << "INVALID COMMAND FROM SOCKET: " << socket_id ;
}


void SocketItem::socketRxData()
{    

    if(socket->bytesAvailable()==0) return;
    QByteArray data = socket->readAll();

    QList<QByteArray> command = decodeCommand(&data);
    if(command.size()) emit execCommandSgn(id, command);

}
void SocketItem::socketTxData(uchar socket_id, QList<QByteArray>  command)
{
    // Only the target client receives the data (or the broadcast)
    if((socket_id !=0 ) && (socket_id != id)) return;
    if(command.size() == 0) return;

    QByteArray frame;

    frame.append("< ");
    for(int i=0; i< command.size(); i++){
        frame.append(command.at(i));
        frame.append(' ');
    }
    frame.append(">\n\r");

    this->socket->write(frame);
    this->socket->waitForBytesWritten(100);
    return;

}

QList<QByteArray> SocketItem::decodeCommand(QByteArray* frame){
    QList<QByteArray> answer;

    if(frame->size() < 5) return answer;
    if(frame->at(0) != '<') return answer;

    QByteArray item;
    bool getItem;

    if(frame->at(1) == ' ') getItem = false;
    else getItem = true;


    int i = 1;
    char cval;

    while(i<frame->size()){
       cval = frame->at(i++);

       if(cval == '>') {
           if(item.size()) answer.append(item);
           break;
       }

       if( (!getItem) && (cval != ' ')){
           getItem = true;
           item.clear();
           item.append(cval);
           continue;
       }

       if( (getItem) && (cval == ' ')){
           getItem = false;
           answer.append(item);
           item.clear();
           continue;
       }

       item.append(cval);
    }
    if(cval != '>') answer.clear();
    return answer;
}
