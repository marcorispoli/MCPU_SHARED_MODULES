#ifndef DEVICE_H
#define DEVICE_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QMutex>
#include <QWaitCondition>

class canRegister{

public:

    explicit canRegister(uchar address){
        for(int i=0; i<4; i++){
            data[i] =0;
        }
        this->address = address;
        changed = false;
        updated = false;
    }
    ~canRegister(){};

    bool set(uchar b0, uchar b1, uchar b2, uchar b3){
        updated = true;
        if((data[0] == b0) && (data[1] == b1) && (data[2] == b2) && (data[3] == b3)){
            changed = false;
        }else{
            data[0] = b0;
            data[1] = b1;
            data[2] = b2;
            data[3] = b3;
            changed = true;
        }
        return changed;
    }

    void update(uchar b0, uchar b1, uchar b2, uchar b3){
        if((data[0] == b0) && (data[1] == b1) && (data[2] == b2) && (data[3] == b3)){
            updated = true;
        }else{
            updated = false;
        }

        return ;
    }

    bool get(uchar* b0, uchar* b1, uchar* b2, uchar* b3){
        *b0 = data[0];
        *b1 = data[1];
        *b2 = data[2];
        *b3 = data[3];
        bool curstat = changed;
        changed = false;
        updated = false;
        return curstat;
    }


    uchar data[4]; // Data written to device
    uchar address;
    bool  changed;
    bool  updated;
};


class Device: public QObject
{
    Q_OBJECT

public:
    explicit Device(ushort CanId, QString IP, uint port);
    ~Device();

    // protocol Implementation
    typedef enum{
        _PROTO_NOT_DEFINED  = 0,
        _PROTO_READ_STATUS  = 1,
        _PROTO_WRITE_PARAM  = 2,
        _PROTO_WRITE_DATA   = 3,
        _PROTO_STORE_PARAMS = 4,
        _PROTO_COMMAND_EXEC = 5,
    }_CanProtocolFrameCode;

    typedef enum{
       _S_RESERVED = 0,
       _S_REVISION,
       _S_SYSTEM,
       _S_ERROR,
       _S_COMMAND,
       _S_LAST
    }_StatusRegisters;

    typedef enum{
       _P_RESERVED = 0,
       _P_LAST
    }_ParamRegisters;

    typedef enum{
       _D_RESERVED = 0,
       _D_LAST
    }_DataRegisters;

    typedef enum {
        _CAN_NO_ERROR =0,
        _CAN_ERROR_TMO,
        _CAN_ERROR_FRAME,
        _CAN_ERROR_NET
    }_CanTxErrors;
signals:


public slots:
    virtual void canDriverReady(bool ){};
    virtual void targetDeviceReady(bool ){};
    virtual void canTxRxCompleted(uchar seq, _CanTxErrors error){};

public:
    _inline canRegister getStatusRegister(uchar address) { return statusRegisters[address];}
    _inline canRegister getParamRegisters(uchar address) { return paramRegisters[address];}
    _inline canRegister getDataRegisters(uchar address) { return dataRegisters[address];}


    int Connect(void);
    int Disconnect(void);
    int Reconnect(void);

protected:
    ushort canId;
    QList<canRegister> statusRegisters;
    QList<canRegister> paramRegisters;
    QList<canRegister> dataRegisters;


private slots:
    void socketRxData(); // Ricezione dati da socket
    void socketError(QAbstractSocket::SocketError error); // Errore dal socket
    void socketConnected(); // Segnale di connessione avvenuta con il server
    void socketDisconnected(); // IL server ha chiiuso la connessione
    void verifyClientConfiguration(void);
    void verifyRevisionCommand(void);

protected:
    QByteArray formatReadStatus(uchar seq, uchar address);
    bool txCanData(QByteArray);

    void setDeviceConnection(bool stat);

protected:
    uchar maj;
    uchar min;
    uchar sub;

private:
    int heartbeatTimer;
    bool deviceConnected;

    void  timerEvent(QTimerEvent* ev);
    int   txCanTimeout;
    bool  tx_pending;
    _CanTxErrors tx_error;
    uint rx_err_cnt;


    // Connection with the Can Server Socket
    QHostAddress serverip;      // Addrees of the remote server
    quint16      serverport;    // Port of the remote server
    QTcpSocket*  socket;        // Socket for  the ethernet client connection to the can Driver
    void clientConnect();       // Try to connect the canDriver
    bool connectionStatus;


    // Driver Configuration
    ushort configuredCanId;

    void configClient(ushort canId);   // Comand to configure the client with the canId
    void readDeviceRevision(void);

    // Can Protocol Handling
    QByteArray rxcanframe;
    QByteArray txcanframe;


    _CanProtocolFrameCode decodeFrame(void);
    bool evaluateReadStatusFrame(void);
    bool evaluateWriteParamFrame(void);
    bool evaluateWriteDataFrame(void);
    bool evaluateStoreFrame(void);
    bool evaluateCommandFrame(void);

    uchar current_sequence;
};

#endif // DEVICE_H
