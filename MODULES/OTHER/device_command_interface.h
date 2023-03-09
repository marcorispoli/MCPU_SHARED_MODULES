#ifndef DEVICE_COMMAND_INTERFACE_H
#define DEVICE_COMMAND_INTERFACE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>


class SocketItem: public QObject
{
     Q_OBJECT

public:

    explicit SocketItem(){
        idcount++;
        if(idcount==0) idcount++;
        id = idcount;
    };

    ~SocketItem(){};

signals:
    void itemDisconnected(ushort id);
    void execCommandSgn(uchar id, QList<QByteArray> command);


public slots:

    void disconnected();
    void socketError(QAbstractSocket::SocketError error);
    void socketRxData();
    void socketTxData(uchar , QList<QByteArray> );

public:
    QTcpSocket* socket;
    uchar id;
    static uchar idcount;

private:
    // Protocol Decoding
    QList<QByteArray> decodeCommand(QByteArray* frame);

};

class deviceCommandInterface : public QTcpServer
{
    Q_OBJECT

public:

    explicit deviceCommandInterface(QString ipaddress, int port);
    ~deviceCommandInterface();

    bool CommandStart(void);

    typedef void (*funcPtr)(deviceCommandInterface* parent, uint id, QList<QByteArray>* command);
    QMap<QString, funcPtr> commandList;
    void sendToClient(uchar id, QList<QByteArray> command){ emit sendToClientSgn(id, command);};

signals:
    void sendToClientSgn(uchar id, QList<QByteArray> command);
    void execCommandSgn(uchar id, QList<QByteArray> command);

private slots:
    void disconnected(ushort id);
    // void execCommandSlot(uchar id, QList<QByteArray> command){ emit execCommandSgn(id, command);}
    void execCommandSlot(uchar id, QList<QByteArray> command);



private:
    //int _inline localPort(){return localport;} //!< Returns the port binded
    //QHostAddress _inline localAddress(){return localip;} //!< returns the IP Address binded
    static const long _DEFAULT_TX_TIMEOUT = 5000;    //!< Default timeout in ms for tx data
    void incomingConnection(qintptr socketDescriptor) override; //!< Incoming connection slot

    QList<SocketItem*>  socketList;    //!< List of Sockets
    QHostAddress        localip;       //!< Address of the local server
    quint16             localport;     //!< Port of the local server

};


#endif // DEVICE_COMMAND_INTERFACE_H
