#ifndef MASTERINTERFACE_H
#define MASTERINTERFACE_H


#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QMutex>
#include <QWaitCondition>
#include <QProcess>

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

    // Protocol position definition
    static const unsigned char ACK_SEQ_CODE = 1;
    static const unsigned char ACK_CMD_CODE = 2;
    static const unsigned char ACK_ERR_CODE = 4;
    static const unsigned char ACK_FIRST_PARAM_CODE = 5;

    static const unsigned char EVENT_CMD = 1;
    static const unsigned char EVENT_FIRST_PARAM_CODE = 2;

    void Start(void);
    virtual void handleReceivedEvent(QList<QString>* event_content);
    virtual void handleReceivedAck(QList<QString>* ack_content);
    virtual void handleServerConnections(bool status);


    static bool startDriver(QString program, QString params, QObject* object);

    _inline bool isConnected(void){ return connectionStatus;};
    _inline bool isAck(void){ return rxack;};
    _inline QList<QString> getAckFrame(void){ return ackparam;};

    _inline bool isReceivedRevision(void){ return revision_is_received;};
    _inline bool isValidRevision(void){ return revision_is_valid;};

    _inline uint getMajRevision(void){ return maj_rev;};
    _inline uint getMinRevision(void){ return min_rev;};
    _inline uint getSubRevision(void){ return sub_rev;};
    _inline uint getMajPkgRevision(void){ return pkg_maj_rev;};
    _inline uint getMinPkgRevision(void){ return pkg_min_rev;};

    void setRevision(uint maj, uint min, uint sub){
        maj_rev = maj;
        min_rev = min;
        sub_rev = sub;
        revision_is_received = true;

        // Checks the packaje expected release
        if( (pkg_maj_rev == maj_rev) && (pkg_min_rev == min_rev) ) revision_is_valid = true;
        else revision_is_valid = false;
    };

    void setPkgRevision(uint maj, uint min){
        pkg_maj_rev = maj;
        pkg_min_rev = min;

        // Checks the packaje expected release
        if( (pkg_maj_rev == maj_rev) && (pkg_min_rev == min_rev) ) revision_is_valid = true;
        else revision_is_valid = false;
    };

protected:
    bool revision_is_valid;
    bool revision_is_received;
    uint maj_rev;
    uint min_rev;
    uint sub_rev;
    uint pkg_maj_rev;
    uint pkg_min_rev;

    void txCommand(QString command, QList<QString>* params = nullptr);

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



    // Protocol data exchange variables
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
