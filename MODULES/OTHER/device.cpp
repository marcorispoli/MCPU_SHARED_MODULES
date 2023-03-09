#include "device.h"
#include <QTimer>
#include <QTimerEvent>

Device::Device(ushort canAddr, QString IP, uint port)
{
    connectionStatus=false;
    serverip = QHostAddress(IP);
    serverport = port;

    // Crea il socket
    socket = new QTcpSocket(this);
    connect(socket,SIGNAL(connected()),this,SLOT(socketConnected()),Qt::UniqueConnection);
    connect(socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)),Qt::UniqueConnection);
    connect(socket,SIGNAL(readyRead()), this,SLOT(socketRxData()),Qt::UniqueConnection);
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnected()),Qt::UniqueConnection);

    this->canId = canAddr;
    configuredCanId = 0;

    // Status register instance
    statusRegisters.append(canRegister(_S_RESERVED));
    statusRegisters.append(canRegister(_S_REVISION));
    statusRegisters.append(canRegister(_S_SYSTEM));
    statusRegisters.append(canRegister(_S_ERROR));
    statusRegisters.append(canRegister(_S_COMMAND));


    // Parameter registers instance
    paramRegisters.append(canRegister(_P_RESERVED));

    // Data registers instance
    dataRegisters.append(canRegister(_D_RESERVED));

    txCanTimeout = 0;
    tx_pending = false;
    rx_err_cnt = 0;

    deviceConnected = false;
    heartbeatTimer = 0;
}

Device::~Device()
{
    if(socket)
    {
        socket->close();
        socket->deleteLater();
        socket=0;
        connectionStatus=false;
    }
}


int Device::Connect(void)
{
    if(connectionStatus)   return 1;
    socket->connectToHost(serverip, serverport);
    return 0;
}

int Device::Reconnect(void)
{
    if(connectionStatus){
        connectionStatus = false;
        socket->abort();
        socket->close();
        socket->connectToHost(serverip, serverport);
        return 0;
    }

    Connect();
    return 0;
}

int Device::Disconnect(void)
{
    if(!connectionStatus) return 0;
    socket->abort();
    socket->close();
    connectionStatus = false;
    return 0;
}


void Device::socketConnected()
{
    // Connessione avvenuta
    connectionStatus=true;
    socket->setSocketOption(QAbstractSocket::LowDelayOption,1);

    // Send the configuration command
    configClient(canId);
    QTimer::singleShot(20,this,SLOT(verifyClientConfiguration()));
}

void Device::verifyRevisionCommand(void){
    if(statusRegisters[_S_REVISION].updated){
       uchar val;
       statusRegisters[Device::_S_REVISION].get(&maj, &min, &sub, &val);
       targetDeviceReady(true);
       setDeviceConnection(true);
       qDebug() << "DEVICE CONNECTED: " << maj << min << sub;

    }else {
        readDeviceRevision();
        QTimer::singleShot(20,this,SLOT(verifyRevisionCommand()));
    }
}

void Device::verifyClientConfiguration(void){
    if(configuredCanId == canId){
        canDriverReady(true);
        QTimer::singleShot(20,this,SLOT(verifyRevisionCommand()));
        qDebug() << "CLIENT CONFIGURED:" << canId;
    }
    else {
        configClient(canId);
        QTimer::singleShot(20,this,SLOT(verifyClientConfiguration()));
    }
}

void Device::socketDisconnected()
{
    connectionStatus=false;
    canDriverReady(false);
    socket->connectToHost(serverip, serverport);
}


void Device::socketError(QAbstractSocket::SocketError error)
{
    // Invia la comunicazione tempestiva che la comunicazione è interrotta
    if(connectionStatus==true)
    {
        connectionStatus=false;
        canDriverReady(false);
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

void Device::socketRxData()
{
    if(connectionStatus ==false) return;
    if(socket->bytesAvailable()==0)
    {
        return;
    }

    rxcanframe = socket->readAll();
    if(rxcanframe.size()==0)
    {
        return;
    }

    // Check the data protocol
    if(rxcanframe.size() < 3) return;
    if(rxcanframe.at(0)!='<') return;
    if(rxcanframe.at(2)!='>') return;

    // Heartbeat Packet
    if(rxcanframe.at(1) == 'H'){

        return;
    }

    // Configuration
    if(rxcanframe.at(1) == 'C'){

        if(rxcanframe.size() < 5) return;
        configuredCanId = (rxcanframe.at(3) + 256 * rxcanframe.at(4)) & 0x7FF;
        return;
    }


    bool evaluation = false;

    switch(decodeFrame()){
    case _PROTO_READ_STATUS:
        evaluation = evaluateReadStatusFrame();
        break;
    case _PROTO_WRITE_PARAM:
        evaluation = evaluateWriteParamFrame();
        break;
    case _PROTO_WRITE_DATA:
        evaluation = evaluateWriteDataFrame();
        break;
    case _PROTO_STORE_PARAMS:
        evaluation = evaluateStoreFrame();
        break;
    case _PROTO_COMMAND_EXEC:
        evaluation = evaluateCommandFrame();
        break;

    default:
        evaluation = false;
        break;

    }

    killTimer(txCanTimeout);
    txCanTimeout = 0;
    tx_pending = false;

    if(evaluation){

        // Reset Heartbeat
        if(heartbeatTimer) killTimer(heartbeatTimer);
        heartbeatTimer = startTimer(5000);

        tx_error = _CAN_NO_ERROR;
        canTxRxCompleted(current_sequence, tx_error);

    }else{
        tx_error = _CAN_ERROR_FRAME;
        rx_err_cnt++;
        canTxRxCompleted(current_sequence, tx_error);
    }

    return;

}

QByteArray Device::formatReadStatus(uchar seq, uchar address){
    QByteArray frame;
    frame.append(seq);
    frame.append((uchar) _PROTO_READ_STATUS);
    frame.append((uchar) address);
    frame.append((uchar) 0);
    frame.append((uchar) 0);
    frame.append((uchar) 0);
    frame.append((uchar) 0);
    frame.append((uchar)seq ^ (uchar) _PROTO_READ_STATUS ^ (uchar) address);
    return frame;
}

Device::_CanProtocolFrameCode Device::decodeFrame(){
    if(rxcanframe.size() < 11) return _PROTO_NOT_DEFINED;
    if(rxcanframe.at(1) != 'D') return _PROTO_NOT_DEFINED;

    uchar crc = 0;
    for(int i=3; i<11; i++) crc ^= (uchar) rxcanframe.at(i);
    if(crc) return _PROTO_NOT_DEFINED;     // Wrong CRC

    if(current_sequence != rxcanframe.at(3)) return _PROTO_NOT_DEFINED;

    return (_CanProtocolFrameCode) rxcanframe.at(4);
}

bool Device::evaluateReadStatusFrame(){

    if(rxcanframe.at(5) >= statusRegisters.size()) return false;
    statusRegisters[rxcanframe.at(5)].set(rxcanframe[6], rxcanframe[7], rxcanframe[8], rxcanframe[9]);
    return true;
}

bool Device::evaluateWriteParamFrame(){
    if(rxcanframe.at(5) >= paramRegisters.size()) return false;
    paramRegisters[rxcanframe.at(5)].update(rxcanframe[6], rxcanframe[7], rxcanframe[8], rxcanframe[9]);
    return true;
}

bool Device::evaluateWriteDataFrame(){
    if(rxcanframe.at(5) >= dataRegisters.size()) return false;
    dataRegisters[rxcanframe.at(5)].update(rxcanframe[6], rxcanframe[7], rxcanframe[8], rxcanframe[9]);
    return true;
}

bool Device::evaluateStoreFrame(){
    return true;
}

bool Device::evaluateCommandFrame(){
    return true;
}


void  Device::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId() == txCanTimeout)
    {
        killTimer(txCanTimeout);
        txCanTimeout = 0;
        tx_pending = false;
        tx_error = _CAN_ERROR_TMO;// Timeout
        rx_err_cnt++;
        canTxRxCompleted(current_sequence, tx_error);
    }

    if(ev->timerId() == heartbeatTimer)
    {
        killTimer(heartbeatTimer);
        heartbeatTimer = 0;
        if(deviceConnected){
            deviceConnected = false;
            targetDeviceReady(false);
        }
    }

}

bool Device::txCanData(QByteArray frame)
{
    if(tx_pending) return false;
    if(frame.size() < 8) return false;
    if(!socket) return false;
    if(!connectionStatus) return false;

    QByteArray data;
    data.append('<');
    data.append('D');
    data.append('>');
    data.append(frame);

    tx_pending = true;
    tx_error = _CAN_NO_ERROR;
    current_sequence = frame.at(0);

    txCanTimeout = startTimer(50); // Start the timeout timer
    socket->write(data);
    socket->waitForBytesWritten(100);

    return true;
}

void Device::configClient(ushort canId)
{
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;

    QByteArray data;
    data.append('<');
    data.append('C');
    data.append('>');
    data.append((uchar) canId);
    data.append((uchar) (canId>>8));


    socket->write(data);
    socket->waitForBytesWritten(100);

}

void Device::readDeviceRevision(void)
{
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;

    QByteArray data;
    data.append('<');
    data.append('D');
    data.append('>');
    data.append(formatReadStatus(0, _S_REVISION));
    txCanData(data);
}

void Device::setDeviceConnection(bool stat)
{
    if(deviceConnected == stat) return;
    deviceConnected = stat;

    if(stat){
        if(heartbeatTimer) killTimer(heartbeatTimer);
        heartbeatTimer = startTimer(5000);
    }else{
        if(heartbeatTimer) killTimer(heartbeatTimer);
        heartbeatTimer = 0;
    }
}

