#ifndef CANCLIENT_H
#define CANCLIENT_H

/*!
 * \defgroup  canClientModule Can Client Driver Communication Module Library.
 *
 * This Library Module implements the connection with the CAN BUS Driver module.
 *
 * # MODULE OVERVIEW
 *
 * This module implements a TcpIp Client Socket Connection based communication,\n
 * with the CAN network Application.
 *
 * The CAN Network Application is an application that allow to access the CAN BUS
 * through the Ethernet.
 *
 * The CAN Application is listening to the address:
 *
 * - IP: 127.0.0.1 (local Host);
 * - PORT: 10001
 *
 * This module allows to:
 * - Connect the CAN applicaiton server;
 * - Set the Acceptance filter mask and the filter address;
 * - Send and receive data to/from CAN bus through the Ethernet.
 *
 *
 * # ETHERNET PROTOCOL DESCRIPTION
 *
 *  There are two only available frames:
 *  - Acceptance Filter frame format;
 *  - Can Data frame format;
 *
 *  Every frame is in ascii format so it can be easily monitored
 *
 *  ## ACCEPTANCE FILTER FRAME FORMAT
 *
 *  The Client needs to open an Acceptance filter in order to receive incoming Can frames.
 *
 *  The rule to accept a CAN frame with a given canId address is:
 *
 *
 *      Acceptance Rule: (canId & filter_mask) == filter_address;
 *
 *  The Acceptance Filter data format is:
 *
 *
 *       <F filter_mask filter_address >
 *
 *  Where
 *  - '<' and '>' are frame delimiters
 *  - F: is the frame type identifier;
 *  - filter_mask: is the 16 bit filter mask;
 *  - filter_address: is the 16 bit filter address;
 *
 *
 *      NOTE: space characters are ignored for the frame syntax;
 *
 *  The filter_mask and filter_address arguments can be:
 *  - Decimal format: example, 1356;
 *  - Hexadecimal format: example, 0x1345
 *
 *  The Acceptance filter workflow is:
 *  - Client sends <F mask address>
 *  - Server answer replying the frame: <F mask address>
 *
 *
 *  ## CAN DATA FRAME FORMAT
 *
 *  The Client can send/Receive data to the BUS using the Can Data Frame format:
 *
 *       <D B0 B1 .. B7>
 *
 *  Where
 *  - '<' and '>' are frame delimiters
 *  - D: is the frame type identifier;
 *  - B0 to B7: are 8 bit can data content.
 *
 *
 *      NOTE: space characters are ignored for the frame syntax;
 *      NOTE: the data lenght can be lower than 8. In this case
 *      the can frame is filled with 0;
 *      NOTE: if the data lenght should be greater than 8, the data will be truncated to 8.
 *
 *  The can data content B0 to B7 can be of the following format:
 *  - Decimal format: example, 125;
 *  - Hexadecimal format: example, 0xCC
 *
 * ## CAN DATA RECEPTION
 *
 * When the CAN Application receives a frame from the Can network, \n
 * it forward that frame to all the clients matching their respective Acceptance filter with the following rule:
 * - (Client_canId & Client_filter_mask) == Client_filter_address;
 *
 * # USAGE
 *
 * Every Application that should send and receive data to the CAN BUS through the
 * Can Application Driver, shall:
 *
 * - instance the Class (or subclass if needed) with the acceptance filter as a parameter;
 * - connect the SIGNAL canClient::rxFromCan() to a local Slot to handle the received can frames;
 * - connect a local signal to the SLOT canClient::txToCanData() to send data to the CAN BUS;
 * - start the tcpIClient connection;
 *
 *
 * \ingroup libraryModules
 *
 */


#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QMutex>
#include <QWaitCondition>

/**
 * @brief The canClient class definition
 *
 */
class canClient: public QTcpServer
{
    Q_OBJECT

public:
    explicit canClient(ushort can_mask, ushort can_address, QString IP, int PORT);
    ~canClient();


    void ConnectToCanServer(void);
    _inline bool isCanReady(void) {return rx_filter_open;}

signals:
    void rxFromCan(ushort canId, QByteArray data);
    void canDriverConnectionStatus(bool status);

public slots:
    void txToCanData(ushort canId, QByteArray data);


private slots:
    void socketRxData(); // Ricezione dati da socket
    void socketError(QAbstractSocket::SocketError error); // Errore dal socket
    void socketConnected(); // Segnale di connessione avvenuta con il server
    void socketDisconnected(); // IL server ha chiiuso la connessione
    void setAcceptanceFilter();

public:
    bool connectionStatus;
    bool connectionAttempt; // E' in corso un tentativo di connessione

private:
    QHostAddress serverip;      // Addrees of the remote server
    quint16      serverport;    // Port of the remote server
    QTcpSocket*  socket;

    ushort filter_mask;         //!< The CAN Acceptance Filter Mask
    ushort filter_address;      //!< The CAN Acceptance Filter Address
    bool rx_filter_open;        //!< The Acceptance filter has been set

    void clientConnect();       // Try to connect the remote server    

    void handleSocketFrame(QByteArray* data);
    ushort getItem(int* index, QByteArray* data, bool* data_ok);
};



#endif // TCPIPCLIENT_H
