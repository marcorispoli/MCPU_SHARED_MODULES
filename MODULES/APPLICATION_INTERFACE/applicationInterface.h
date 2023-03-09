#ifndef APPLICATION_INTERFACE_H
#define APPLICATION_INTERFACE_H


/*!
 * \defgroup  ApplicationInterfaceModule Application Interface Library Module.
 *
 * This Library Module implements the Server communication protocol.
 *
 * The Server communication protocol is used by any application to provides
 * a Command/Events interface layer:
 *
 *  - The Client connecting the application, requests for command execution;
 *  - The Application acknowledge a received command;
 *  - The Application sends to all the connected clients Events frame
 *
 * # CONNECTION DETAILS
 *
 * The Server is assigned to a unique IP and port, defined by the Module that subclass this library.
 *
 * More than one connection can be established at the same time:
 *  - Every connected client is linked to a unique ID identifier;
 *  - The Application will acknowledge a command only to the Sender of the command;
 *  - Every internal EVENT will be forwarded to all the client connected.
 *
 *
 * # PROTOCOL DESCRIPTION
 *
 * The protocol is based on the scheme COMMAND / EVENT / ACK:
 * - The COMMAND is a frame send by a remote Client, requesting for a command execution;
 * - The ACK is the frame the Applicaiton sends back to the Client to acknowledge a received command frame;
 * - The EVENT is a frame that the Application sends to the Client to notify an internal status change. No Acknowledge is needed;
 *
 * The COMMAND Frame is the following:
 * - <C SEQ Command PARAMS... >
 *      - SEQ: is an arbitrary sequence number set by the sender and acknowledge by the receiver;
 *      - Command is the String identifying the command to be executed;
 *      - PARAMS are arbitrary parameters
 *
 * The ACK Frame is
 * - <A SEQ OK/NOK CODE PARAMS... >
 *      - SEQ: is the sequence number of the COMMAND frame to be acknowledge;
 *      - OK/NOK is a mandatory data that identifies if the command can be executed
 *      - CODE: is the an integer String defining an error code (if different from 0).
 *      - PARAMS are optional parameters
 *
 *The EVENT Frame is the following:
 * - <E Event PARAMS... >
 *      - Event: is the identifier of the EVENT;
 *      - PARAM1 to PARAMN are otional parameters;
 *
 * \ingroup libraryModules
 *
 */
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>


/**
 * @brief This Class implements a TcpIp Serve socket, running in a separate thread.
 *
 * The Class implements the basic routines to send and receive data from/to Gantry.
 *
 * \ingroup ApplicationInterfaceModule
 */
class SocketItem: public QObject
{
     Q_OBJECT

public:

    explicit SocketItem(){};
    ~SocketItem(){};

signals:
    void itemDisconnected(ushort id); //!< Signal to inform the system about the communication status.
    void receivedCommandSgn(ushort id, QByteArray data); //!< For every decoded command received the server sends this signal to the Application.


public slots:
    void disconnected(); //!< Disconnection Callback received from the Socket Library
    void socketError(QAbstractSocket::SocketError error); //!< Error callback received from the Library
    void socketRxData(); //!< Data received callback received from the Socket Library
    void socketTxData(QByteArray);//!< Data to be sent to the Socket, received from the Application thread.

public:
    QTcpSocket* socket; //!< Socket pointer
    ushort id;  //!< Unique ID of the Connected Client
};

/**
 * @brief This class resides into the Main Thread and implements the
 * communication protocol with the Gantry.
 *
 * The class starts listening an incoming Client connection and creates\n
 * for any individual connection a given socket handler (SocketItem class). \n
 * Every Socket is assigned to a unique ID so that the Server can redirect the
 * answer frame with the sender client.
 *
 * The Reception and Transmission are implemented into the  QTcpServer library
 * that runs in a separate thread and exchanges data only through the SIgnal/Slot
 * mechanism.
 *
 *
 * \ingroup ApplicationInterfaceModule
 */
class applicationInterface : public QTcpServer
{
    Q_OBJECT

public:

    explicit applicationInterface(QString ipaddress, int port);
    ~applicationInterface();

    static const long _DEFAULT_TX_TIMEOUT = 5000;    //!< Default timeout in ms for tx data
    bool Start(void); /// Starts the Server thread


    virtual uint handleReceivedCommand(QList<QString>* frame, QList<QString>* answer); //!< The Subclass shall implement its own handler for the received commands

signals:
    void txFrame(QByteArray data); /// This signal is Queued connected with the transmitting thread

public slots:
    void receivedCommandSlot(ushort id, QByteArray data); /// This is the slot handling the EVENTs from Gantry
    void disconnected(ushort id); /// Disconnect callback event of tyhe Socket thread


protected:
    void incomingConnection(qintptr socketDescriptor) override; //!< Incoming connection slot
    void sendEvent(QString Event, QList<QString>* params);              //!< Helper function to send an EVENT frame to gantry

private:

    QList<SocketItem*>  socketList;    //!< List of Sockets
    QHostAddress        localip;       //!< Address of the local server
    quint16             localport;     //!< Port of the local server
    ushort              idseq;         //!< Id counter, to assign a unique ID to a client

    void sendAck(ushort id, ushort seq, uint code,QList<QString>*  data);  //!< Helper function to send an Answer frame to Gantry
    QList<QString> getProtocolFrame(QByteArray* data);  //!< Extract the data content from a received frame

};

#endif // SERVER_H
