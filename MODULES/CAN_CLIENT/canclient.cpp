#include "canclient.h"
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
canClient::canClient(ushort can_mask, ushort can_address, QString IP, int PORT):QTcpServer()
{
    serverip = QHostAddress(IP);
    serverport = PORT;
    filter_mask = can_mask;
    filter_address = can_address;
    rx_filter_open = false;
    connectionStatus=false;
    socket=0;

}

/**
 * This is the class distructor.
 *
 * It only destroy the TcpIp socket.
 */
canClient::~canClient()
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
void canClient::ConnectToCanServer(void)
{
    // The Connection process has already been initiated
    if(socket) return ;

    if(connectionStatus){
        qDebug() << "ConnectToCanServer() command failed: The Can Server is already connected!";
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
void canClient::socketConnected()
{
    // Connessione avvenuta
    connectionStatus=true;
    rx_filter_open = false;
    socket->setSocketOption(QAbstractSocket::LowDelayOption,1);

    // Open the Acceptance filter to the Can Server application
    setAcceptanceFilter();

}

/**
 * This is the TcpIp socket callback of the disconnection event.
 *
 * In case of the disconnection, the Function start again a new connection attempt.
 */
void canClient::socketDisconnected()
{
    rx_filter_open = false;
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
void canClient::socketError(QAbstractSocket::SocketError error)
{
    // Invia la comunicazione tempestiva che la comunicazione è interrotta
    if(connectionStatus==true)
    {
        connectionStatus=false;
        rx_filter_open = false;
    }

    if(error == QAbstractSocket::RemoteHostClosedError)
    {
        // Quest'errore viene generato PRIMA della chiusura
        // del socket (slot socketDisconnected())
        rx_filter_open = false;
        return ;
    }


    socket->abort();
    socket->close();
    socket->connectToHost(serverip, serverport);
    return;
}

/**
 * This function get an Item from the tcpIp received frame.
 *
 * This function is used to decode an incoming data frame
 * from the Can Application Driver.
 *
 * @param
 * - index: this is the current character index of the received ethernet frame;
 * - data: this is the received ethernet frame data array;
 * - data_ok: this is a pointer that the caller provides to have the result of the item identification;
 *
 * @return always returns 0;
 *
 */
ushort canClient::getItem(int* index, QByteArray* data, bool* data_ok){
    *data_ok = false;
    bool is_hex = false;
    QString val;

    for(; *index< data->size(); (*index)++) if(data->at(*index) != ' ') break; // Removes the spaces

    for(; *index< data->size(); (*index)++){
        if(data->at(*index) == ' '){
            if(!val.size()) return 0;
            *data_ok = true;
            if(is_hex) return val.toUShort(data_ok, 16);
            else return val.toUShort();
        }
        if((data->at(*index) == 'x') |(data->at(*index) == 'X')) is_hex = true;
        val.append(data->at(*index));
    }

    // Non è terminato con uno spazio: errore
    return 0;
}

/**
 * This is the handler of every received frame.
 *
 * The handler decodes the frame format, providing:
 * - the Acceptance Filter acknowledge;
 * - the CAN DATA received from the can Application Driver;
 *
 * The Handler will emit the canClient::rxFromCan() SIGNAL
 * for every correct can frame received.
 *
 *
 *  NOTE: only the can frame with the canId matching the rule
 *  of the acceptance filter will generate the Signal.
 *
 *
 * @param data
 */
void canClient::handleSocketFrame(QByteArray* data){

    QByteArray frame;
    bool is_register;
    bool is_valid;
    int i;
    bool data_ok;


    is_valid = false;
    for(i=0; i< data->size(); i++){
        if(data->at(i)== ' ') continue;
        if(data->at(i)== 'F') {
            is_register = true;
            is_valid = true;
            i++;
            break;
        }

        if(data->at(i)== 'D') {
            is_register = false;
            is_valid = true;
            i++;
            break;
        }
    }
    if(!is_valid) return;

    ushort mask, address;
    if(is_register){// Can Registering Frame: set the reception mask and address
        mask = getItem(&i, data, &data_ok);
        if(!data_ok) return;
        address = getItem(&i, data, &data_ok);
        if(!data_ok) return;
        if(mask != filter_mask) return;
        if(address != filter_address) return;
        qDebug() << QString("ACCEPTANCE FILTER OPEN TO: MASK=0x%1, ADDR=0x%2").arg(filter_mask,1,16).arg(filter_address,1,16);
        rx_filter_open = true;
        return;

    }else{


        frame.clear();
        ushort canid = getItem(&i, data, &data_ok);        
        if(!data_ok) return;

        ushort val;
        for(; (i< data->size()) && (frame.size() <= 8) ; i++){
            val = getItem(&i, data, &data_ok)&0xFF;
            if(data_ok) {
                frame.append((unsigned char) val);
            } else break;
        }

        // If a valid set of data has been identified they will be sent to the driver
        //ushort devId = (canid & (~filter_mask) );
        if(frame.size()) {
            emit rxFromCan(canid,frame);
        }
    }
    return;
}

/**
 * This callback is called every received data frame from the ethernet.
 *
 * A Valid data frame is determined when the initiator and terminator\n
 * characters are identified (as for the protocol spec):
 * - initiator = '<';
 * - terminator = '>';
 *
 * A valid data frame is < .... >.
 *
 * The function is able to identify nested valid frames: < frame 1 >  <frame 2>.
 *
 * For every nested frame the function will call the canClient::handleSocketFrame() method.
 */
void canClient::socketRxData()
{
    if(connectionStatus ==false) return;
    if(socket->bytesAvailable()==0) return;
    QByteArray data = socket->readAll();
    QByteArray frame;

    // Identifies all the possible frames in the received stream
    for(int i=0; i<data.size(); i++){
        if(data.at(i) == '<') {
            frame.clear();
        }else if(data.at(i) == '>'){
            if(frame.size() > 4) {
                frame.append(' ');
                handleSocketFrame(&frame);
                frame.clear();
            }
        }else{
            frame.append(data.at(i));
        }
    }

}

/**
 * This is the Slot that sends data to the Can  Application Driver
 * with the proper data format.
 *
 * This Slot shall be connected to a local signal emitting a can data frame
 * to be sent to the Can Application Driver.
 *
 * @param
 * - canId: this is the 11 bit Target Can ID;
 * - data: this is the can data load. Max 8 bytes are admitted.
 */
void canClient::txToCanData(ushort canId, QByteArray data)
{
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;


    QString frame = QString("<D %1 ").arg(canId);
    for(int i=0; i<data.size(); i++) frame.append(QString("%1 ").arg((unsigned char) data.at(i)));
    frame.append(">");


    socket->write(frame.toLatin1());
    socket->waitForBytesWritten(5000);


}

/**
 * This is the slot function that is resceduled
 * every 50 ms until the Can application Driver will
 * acknowledge the correct Acceptance Filter registration.
 *
 */
void canClient::setAcceptanceFilter()
{
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;
    if(rx_filter_open) return;

    QString frame = QString("<F %1 %2>").arg(filter_mask).arg(filter_address);
    socket->write(frame.toLatin1());
    socket->waitForBytesWritten(5000);

    QTimer::singleShot(50,this, SLOT(setAcceptanceFilter()));
}
