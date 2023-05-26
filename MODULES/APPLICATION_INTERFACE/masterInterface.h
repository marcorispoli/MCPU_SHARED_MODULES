#ifndef MASTERINTERFACE_H
#define MASTERINTERFACE_H


#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QMutex>
#include <QWaitCondition>

/**
 * @brief The masterInterface class definition
 *
 */
class masterInterface: public QTcpServer
{
    Q_OBJECT

public:
    explicit masterInterface(QString IP, int PORT);
    ~masterInterface();

    const unsigned char ACK_ERR_CODE = 3;
    const unsigned char ACK_FIRST_PARAM_CODE = 4;
    const unsigned char EVENT_CODE = 1;
    const unsigned char EVENT_FIRST_PARAM_CODE = 2;

    void Start(void);
    virtual void handleReceivedEvent(QList<QString>* event_content);
    void txCommand(QString command, QList<QString>* params);

    _inline bool isConnected(void){ return connectionStatus;};
    _inline bool isAck(void){ return rxack;};
    _inline QList<QString> getAckFrame(void){ return ackparam;};


private slots:
    void socketRxData(); // Ricezione dati da socket
    void socketError(QAbstractSocket::SocketError error); // Errore dal socket
    void socketConnected(); // Segnale di connessione avvenuta con il server
    void socketDisconnected(); // IL server ha chiiuso la connessione

private:
    QHostAddress serverip;      // Addrees of the remote server
    quint16      serverport;    // Port of the remote server
    QTcpSocket*  socket;
    bool connectionStatus;
    bool connectionAttempt; // E' in corso un tentativo di connessione

    uint txseq;
    uint rxseq;
    bool rxack;
    uint acktmo;
    QList<QString> ackparam;

    void clientConnect();       // Try to connect the remote server    
    void handleSocketFrame(QByteArray* data);
    QList<QString> getProtocolFrame(QByteArray* data);
};



#endif // TCPIPCLIENT_H
