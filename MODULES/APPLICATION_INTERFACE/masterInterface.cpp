#include "masterInterface.h"
#include <QTimer>
#include <QProcess>
#include <QFile>

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
masterInterface::masterInterface(QString debugName, QString program, QString progpar, QString IP, int PORT):QTcpServer()
{
    debugProcessName = debugName;
    serverip = QHostAddress(IP);
    serverport = PORT;
    connectionStatus=false;
    socket=0;
    boardInitialized = false;
    board_revision_is_valid = false;;
    revision_is_received = false;
    revision_is_valid = false;
    pkg_maj_rev = 0;
    pkg_min_rev = 0;

    // Test if the driver process is present
    QFile programma(program);
    if(!programma.exists()){
        qDebug() << "PROCESS: " << program << " doesn't exist";
        driverProcess = nullptr;
    }else{

        // Create the process handler
        driverProcess = new QProcess(this);
        driverProcess->setProgram(program);
        QStringList arguments;
        arguments.append(progpar);
        driverProcess->setArguments(arguments);

    }

}

/**
 * This is the class distructor.
 *
 * It only destroy the TcpIp socket.
 */
masterInterface::~masterInterface()
{
    if(socket)
    {
        socket->close();
        socket->deleteLater();
        socket=0;
        connectionStatus=false;
    }

    if(driverProcess) {
        driverProcess->kill();
    }
}

/**
 * This Method starts the connection with the Can Application Driver.
 *
 */
void masterInterface::Start(void)
{
    // The Connection process has already been initiated
    if(socket) return ;

    if(connectionStatus){
        qDebug() << "Start() command failed: the socket is already connected!";
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
void masterInterface::socketConnected()
{
    // Connessione avvenuta
    connectionStatus=true;
    revision_is_received = false;
    revision_is_valid = false;
    socket->setSocketOption(QAbstractSocket::LowDelayOption,1);
    handleServerConnections(connectionStatus);

}

/**
 * This is the TcpIp socket callback of the disconnection event.
 *
 * In case of the disconnection, the Function start again a new connection attempt.
 */
void masterInterface::socketDisconnected()
{
    connectionStatus=false;
    revision_is_received = false;
    revision_is_valid = false;
    socket->connectToHost(serverip, serverport);
    handleServerConnections(connectionStatus);
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
void masterInterface::socketError(QAbstractSocket::SocketError error)
{
    // Invia la comunicazione tempestiva che la comunicazione è interrotta
    if(connectionStatus==true)
    {
        connectionStatus=false;
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



/**
 * @brief getProtocolFrame
 *
 * This function extract a list of String Items contained into
 * the parameter. Those items are parts of the Command frame.
 *
 * Each individual item is separated with a space from the next Item,
 * with the exception of the first and the last item:
 * - the first item can be preceded by the '<' character.
 * - the last item can be followed by the '>' character.
 *
 *
 * @param
 * - data: dataframe containing the list of command items;
 *
 * @return
 * - The list of the Items composing the frame
 *
 */
QList<QString> masterInterface::getProtocolFrame(QByteArray* data){
    QList<QString> comando;

    bool init_find = true;
    QString stringa = "";

    for(int i=0; i<data->size(); i++){
        if(init_find){
            if(data->at(i) != ' '){
                stringa = data->at(i);
                init_find = false;
            }
        }else{
            if(data->at(i) == ' '){
                comando.append(stringa);
                init_find = true;
                stringa = "";
            }else stringa += data->at(i);
        }
    }
    if(stringa != "") comando.append(stringa);
    return comando;
}



void masterInterface::handleSocketFrame(QByteArray* data){

    // Extracts the protocol items list
    QList<QString> frame_content =  getProtocolFrame(data);
    if(frame_content.size() < 3) return;



    if(frame_content.at(0) == "A") {
         if(frame_content.at(1).toUShort() != txseq) return;
         if(rxack) return;

         ackparam = frame_content;
         rxack = true;
         handleLibReceivedAck(&frame_content);
         return;
    }else if(frame_content.at(0) == "E") {
        handleLibReceivedEvent(&frame_content);
        return;
    }

    return;
}


void masterInterface::socketRxData()
{

    QByteArray frame;
    QString stringa;

    if(connectionStatus ==false) return;
    if(socket->bytesAvailable()==0) return;
    QByteArray data = socket->readAll();


    bool init_find = true;
    QByteArray streaming;
    streaming.clear();

    for(int i=0; i<data.size(); i++){
        if(init_find){
            if(data.at(i) == '<'){
                streaming.clear();
                init_find = false;
            }
        }else{
            if(data.at(i) == '>'){
                if(streaming.size()) handleSocketFrame(&streaming);
                streaming.clear();
                init_find = true;
            }else streaming.append(data.at(i));
        }

    }

}

void masterInterface::txCommand(QString command, QList<QString>* params)
{
    static ushort loc_txseq = 1;
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;

    QByteArray buffer;

    // Creates the Command frame
    buffer.append('<');
    buffer.append('C');
    buffer.append(' ');
    buffer.append(QString("%1").arg(loc_txseq).toLatin1());
    buffer.append(' ');
    buffer.append(command.toLatin1());
    buffer.append(' ');
    if(params != nullptr){
        // Append the data content of the frame
        for(int i =0; i< params->size(); i++){
                buffer.append(params->at(i).toLatin1());
                buffer.append(' ');
        }
    }

    buffer.append('>');

    // Append a line feed to help a character client to show the frame
    buffer.append('\n');
    buffer.append('\r');

    txseq = loc_txseq++;
    rxack = false;


    socket->write(buffer);
    socket->waitForBytesWritten(100);


}

bool masterInterface::startDriver(void){

    if(!driverProcess) return false;
    if(driverProcess->state() != QProcess::NotRunning ) return true;
    driverProcess->start();

    bool result = driverProcess->waitForStarted(5000);
    if(!result) {
        qDebug() << "PROCESS: " << progname << " not started!";
        return false;
    }

    return true;
}

void  masterInterface::stopDriver(void){

    if(driverProcess) {
        driverProcess->kill();
    }
}



void masterInterface::handleLibReceivedEvent(QList<QString>* event_content){
    if(event_content->at(EVENT_CMD) == EVENT_INIT_COMPLETED){
        if(event_content->size() != EVENT_INIT_COMPLETED_LEN) return;

        boardInitialized = true;
        bootloader_error = event_content->at(EVENT_FIRST_PARAM_CODE).toUInt();
        bootloader_present = event_content->at(EVENT_FIRST_PARAM_CODE+1).toUInt();
        bootloader_running = event_content->at(EVENT_FIRST_PARAM_CODE+2).toUInt();
        boardAppMaj = event_content->at(EVENT_FIRST_PARAM_CODE+3).toUInt();
        boardAppMin = event_content->at(EVENT_FIRST_PARAM_CODE+4).toUInt();
        boardAppSub = event_content->at(EVENT_FIRST_PARAM_CODE+5).toUInt();
        bootloaderMaj = event_content->at(EVENT_FIRST_PARAM_CODE+6).toUInt();
        bootloaderMin = event_content->at(EVENT_FIRST_PARAM_CODE+7).toUInt();
        bootloaderSub = event_content->at(EVENT_FIRST_PARAM_CODE+8).toUInt();

        if(bootloader_error) qDebug() << "POWERSERVICE BOARD STATUS: BOOTLOADER ERROR " << bootloader_error;
        else{
            qDebug() << debugProcessName  << " BOARD STATUS: BOOTPRESENT->" << bootloader_present \
                     << " BOOTRUN->" << bootloader_running \
                     << " BOOTREV->" << bootloaderMaj <<"."<<bootloaderMin<<"."<<bootloaderSub \
                     << " APPREV->" << boardAppMaj <<"."<<boardAppMin<<"."<<boardAppSub ;
        }

        return;
    }

    handleReceivedEvent(event_content);
    return;
}

void masterInterface::handleReceivedEvent(QList<QString>* event_content){

    return;
}

void masterInterface::handleLibReceivedAck(QList<QString>* ack_content){
    if(ack_content->at(ACK_CMD_CODE) == GET_REVISION){

        if(ack_content->size() != GET_REVISION_LEN) return;
        setRevision(ack_content->at(ACK_FIRST_PARAM_CODE).toUInt(), ack_content->at(ACK_FIRST_PARAM_CODE+1).toUInt(), ack_content->at(ACK_FIRST_PARAM_CODE+2).toUInt());
        qDebug() << debugProcessName  <<  " REVISION: " << maj_rev << "." << min_rev << "." << sub_rev;
        return;
    }

    if(ack_content->at(ACK_CMD_CODE) == BOARD_INIT){

        if(ack_content->size() != BOARD_INIT_LEN) return;
        qDebug() << debugProcessName  << " BOARD INIT ACK";
        return;
    }

    // If no lib management are performed proceeds with the module management
    handleReceivedAck(ack_content);
    return;
}

void masterInterface::handleReceivedAck(QList<QString>* ack_content){


    return;
}

void masterInterface::handleServerConnections(bool status){


    return;
}
