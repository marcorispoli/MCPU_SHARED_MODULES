#include "masterInterface.h"
#include <QTimer>

/**
 * This is the class constructor.
 *
 * The Constructor only initializes some internal variable.
 *
 * @param
 * - can_mask: this is the 16 bit Acceptance Mask;
 * - can_address: this is the 16 bit acceptance filter;
 * - IP: this is the IP address of the Can Application Driver;
 * - PORT: this is the port of the Can Application Driver;
 */
masterInterface::masterInterface(QString IP, int PORT):QTcpServer()
{
    serverip = QHostAddress(IP);
    serverport = PORT;
    connectionStatus=false;
    socket=0;

}

/**
 * This is the class distructor.
 *
 * It only destroy the TcpIp socket.
 */
masterInterface::~masterInterface()
{
    if(socket)
    {
        socket->close();
        socket->deleteLater();
        socket=0;
        connectionStatus=false;
    }
}

/**
 * This Method starts the connection with the Can Application Driver.
 *
 */
void masterInterface::Start(void)
{
    // The Connection process has already been initiated
    if(socket) return ;

    if(connectionStatus){
        qDebug() << "Start() command failed: the socket is already connected!";
        return ;
    }


    // Crea il socket
    socket = new QTcpSocket(this);
    connect(socket,SIGNAL(connected()),this,SLOT(socketConnected()),Qt::UniqueConnection);
    connect(socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)),Qt::UniqueConnection);
    connect(socket,SIGNAL(readyRead()), this,SLOT(socketRxData()),Qt::UniqueConnection);
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnected()),Qt::UniqueConnection);

    socket->connectToHost(serverip, serverport);
    return ;
}

/**
 * This is the TcpIp socket callback when the Ethernet connection has been established.
 *
 * As soon as the connection is established, the function starts the Acceptance Filter
 * registration process.
 *
 */
void masterInterface::socketConnected()
{
    // Connessione avvenuta
    connectionStatus=true;
    socket->setSocketOption(QAbstractSocket::LowDelayOption,1);

}

/**
 * This is the TcpIp socket callback of the disconnection event.
 *
 * In case of the disconnection, the Function start again a new connection attempt.
 */
void masterInterface::socketDisconnected()
{
    connectionStatus=false;
    socket->connectToHost(serverip, serverport);

}

/**
 * This is the TcpIp Socket connection error callback.
 *
 * In case of socket error, the socket is closed and a new
 * connection is activated.
 *
 * @param
 * - error: this is the error code received from the tcpIp Socket handler.
 */
void masterInterface::socketError(QAbstractSocket::SocketError error)
{
    // Invia la comunicazione tempestiva che la comunicazione è interrotta
    if(connectionStatus==true)
    {
        connectionStatus=false;
    }

    if(error == QAbstractSocket::RemoteHostClosedError)
    {
        // Quest'errore viene generato PRIMA della chiusura
        // del socket (slot socketDisconnected())       
        return ;
    }


    socket->abort();
    socket->close();
    socket->connectToHost(serverip, serverport);
    return;
}



/**
 * @brief getProtocolFrame
 *
 * This function extract a list of String Items contained into
 * the parameter. Those items are parts of the Command frame.
 *
 * Each individual item is separated with a space from the next Item,
 * with the exception of the first and the last item:
 * - the first item can be preceded by the '<' character.
 * - the last item can be followed by the '>' character.
 *
 *
 * @param
 * - data: dataframe containing the list of command items;
 *
 * @return
 * - The list of the Items composing the frame
 *
 */
QList<QString> masterInterface::getProtocolFrame(QByteArray* data){
    QList<QString> comando;

    bool init_find = true;
    QString stringa = "";

    for(int i=0; i<data->size(); i++){
        if(init_find){
            if(data->at(i) != ' '){
                stringa = data->at(i);
                init_find = false;
            }
        }else{
            if(data->at(i) == ' '){
                comando.append(stringa);
                init_find = true;
                stringa = "";
            }else stringa += data->at(i);
        }
    }
    if(stringa != "") comando.append(stringa);
    return comando;
}


void masterInterface::handleReceivedEvent(QList<QString>* event_content){
    return;
}

void masterInterface::handleSocketFrame(QByteArray* data){

    // Extracts the protocol items list
    QList<QString> frame_content =  getProtocolFrame(data);
    if(frame_content.size() < 3) return;

    if(frame_content.at(0) == "A") {
         if(frame_content.at(1).toUShort() != txseq) return;
         if(rxack) return;

         ackparam = frame_content;
         rxack = true;
         return;
    }else if(frame_content.at(0) == "E") {
        handleReceivedEvent(&frame_content);
        return;
    }

    return;
}


void masterInterface::socketRxData()
{

    QByteArray frame;
    QString stringa;

    if(connectionStatus ==false) return;
    if(socket->bytesAvailable()==0) return;
    QByteArray data = socket->readAll();


    bool init_find = true;
    QByteArray streaming = "";

    for(int i=0; i<data.size(); i++){
        if(init_find){
            if(data.at(i) == '<'){
                streaming.clear();
                init_find = false;
            }
        }else{
            if(data.at(i) == '>'){
                if(streaming.size()) handleSocketFrame(&streaming);
                streaming.clear();
                init_find = true;
            }else streaming.append(data.at(i));
        }

    }

}

void masterInterface::txCommand(QString command, QList<QString>* params)
{
    static ushort loc_txseq = 1;
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;

    QByteArray buffer;

    // Creates the Command frame
    buffer.append('<');
    buffer.append('C');
    buffer.append(' ');
    buffer.append(QString("%1").arg(loc_txseq).toLatin1());
    buffer.append(' ');
    buffer.append(command.toLatin1());
    buffer.append(' ');
    if(params){
        // Append the data content of the frame
        for(int i =0; i< params->size(); i++){
                buffer.append(params->at(i).toLatin1());
                buffer.append(' ');
        }
    }

    buffer.append('>');

    // Append a line feed to help a character client to show the frame
    buffer.append('\n');
    buffer.append('\r');

    socket->write(buffer);
    socket->waitForBytesWritten(5000);


}
