#include "tcpipserver.h"



TcpIpServer::TcpIpServer(QHostAddress ipaddress, int port):QTcpServer()
{
    socket = nullptr;
    connection_status = false;
    localip = ipaddress;
    localport = port;
}

TcpIpServer::~TcpIpServer()
{
    if(socket != nullptr)
    {
        disconnected();
        connection_status=false;
    }
}

bool TcpIpServer::Start(void)
{
    // Find the ip address match from the system network
    QList<QNetworkInterface> interfacesList = QNetworkInterface::allInterfaces();    
    for(int i =0; i<interfacesList.size(); i++)
    {
        QList<QHostAddress> addressList = interfacesList.at(i).allAddresses();
        // Find the address from the network available addresses
        for(int j=0; j < addressList.size(); j++){
            if(addressList[j] == localip){
                if (!this->listen(localip,localport)) return false;
                return true;
            }
        }
    }
    return false;
}


void TcpIpServer::incomingConnection(qintptr socketDescriptor)
{
    // Any incoming connection replaces a current connected socket
    if(socket != nullptr) disconnected();


    // Create a new socket
    socket = new QTcpSocket(this);
    if(!socket->setSocketDescriptor(socketDescriptor))
    {        
        delete socket;
        return;
    }
    socket->setSocketOption(QAbstractSocket::LowDelayOption,1);


    // Interface signal connection
    connect(socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)),Qt::UniqueConnection);
    connect(socket,SIGNAL(disconnected()),this, SLOT(disconnected()),Qt::UniqueConnection);
    connect(socket,SIGNAL(readyRead()), this, SLOT(socketRxData()),Qt::UniqueConnection);

    // Emit the connection status to the client
    connection_status = true;
    emit serverConnection(true);

    return;
 }



void TcpIpServer::disconnected()
{
    connection_status = false;
    emit serverConnection(false);
    socket->close();
    socket->deleteLater();
    socket = nullptr; // Consente di poter accettare un nuovo client
}


void TcpIpServer::socketError(QAbstractSocket::SocketError error)
{

    if(connection_status) disconnected();
    return;
}



void TcpIpServer::socketRxData()
{

    if(socket->bytesAvailable()==0) return;
    QByteArray data = socket->readAll();
    if(data.size()==0) return;
    emit rxData(data);
}


void TcpIpServer::txData(QByteArray data, long timeout)
{
    if( connection_status == false) return;
    if( socket == nullptr) return;
    socket->write(data);
    socket->waitForBytesWritten(timeout);
}


