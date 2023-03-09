#ifndef TCPIPSERVER_H
#define TCPIPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>

/**
 * @brief This class handles Tcp Ip server connections
 *
 * This class allow to open a port on the ethernet networks.
 *
 * The class allows to heither open a local host port or
 * a port of a network with a given IP address.
 *
 * The socket accepts only one connection at a time:
 * however an incoming connection replaces a current connected connection.
 *
 * # INTERFACE METHODS
 * The class implements the following methods to operate:
 * + TcpIpServer::txData(): sends data to the client;
 * + TcpIpServer::rxData() signal: reception data signal;
 * + TcpIpServer::localPort() : returns the port binded;
 * + TcpIpServer::localAddress() : returns the IP address of the used network;
 * + TcpIpServer::isConnected() : returns the current connection status;
 *
 * # USAGE
 * + Instance the class providing the IP address or Local Host address
 * and the port:
 * \verbatim
      Case with a given IP address:
      pTcpIpServer = new TcpIpServer( QHostAddress("192.168.30.10"), 10006);

      Case with a local host:
      pTcpIpServer = new TcpIpServer( QHostAddress(QHostAddress::LocalHost), 10006);
    \endverbatim
 * + connect the proper signals and slots:
 * \verbatim
    Connect the receiving data slot:
    connect(pTcpIpServer,SIGNAL(rxData(QByteArray)),this,SLOT(tcpIpServerRxData(QByteArray)),Qt::UniqueConnection);

    Connect the connection status slot:
    connect(pTcpIpServer,SIGNAL(serverConnection(bool)),this,SLOT(tcpIpServerConnection(bool)),Qt::UniqueConnection);
   \endverbatim
 * + Start the reception thread:
 * \verbatim
   pTcpIpServer->Start();
   \endverbatim
 *
 */
class TcpIpServer : public QTcpServer
{
    Q_OBJECT

public:
    /**
     * @brief Class Constructor
     * @param ipaddress QHostAddress ipaddress network target
     * The ipaddress can be a IPV4 address or QHostAddress::LocalHost
     * @param port is the port number
     */
    explicit TcpIpServer(QHostAddress ipaddress, int port);
    ~TcpIpServer();

    static const long _DEFAULT_TX_TIMEOUT = 5000;    //!< Default timeout in ms for tx data

    /**
     * @brief Start socket listening
     * @return
     * + true if the network is up and the listening started;
     * + false if the action fails.
     */
    bool Start(void);

    int _inline localPort(){return localport;} //!< Returns the port binded
    QHostAddress _inline localAddress(){return localip;} //!< returns the IP Address binded
    bool _inline isConnected(void) {return connection_status;} //!< Returns the crrent connction status

public slots:
    /**
     * @brief Tx Data sending method
     * @param data
     * QByteArray data to be sent
     * @param timeout
     * (optionnal) is the tx waiting time in ms.
     * The default is TcpIpServer::_DEFAULT_TX_TIMEOUT
     */
    void txData(QByteArray data, long timeout = TcpIpServer::_DEFAULT_TX_TIMEOUT);


protected:
    void incomingConnection(qintptr socketDescriptor) override; //!< Incoming connection slot

signals:
    void serverConnection(bool status);//!< signal for connection change state
    void rxData(QByteArray data); //!< signals for reception data handling

private slots:
    void socketRxData(); //!< Rx data received from the socket
    void disconnected(); //!< Slot to handle the disconnection signal from the socket
    void socketError(QAbstractSocket::SocketError error); //!< Slot to handle error signals from the socket


private:
    bool connection_status;     //!< Connection status
    QTcpSocket*  socket;        //!< Socket pointer
    QHostAddress localip;       //!< Address of the local server
    quint16      localport;     //!< Port of the local server
};



#endif // TCPIPSERVER_H
