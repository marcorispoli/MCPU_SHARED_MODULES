#include "applicationInterface.h"

/**
 * @brief applicationInterface::applicationInterface
 *
 * @param
 * - ipaddress: IP where server will be bounded;
 * - port: bounding port
 *
 */
applicationInterface::applicationInterface(QString ipaddress, int port):QTcpServer()
{
    socketList.clear();
    localip = QHostAddress(ipaddress);
    localport = port;
    idseq=0;
}

/**
 * @brief applicationInterface::~applicationInterface
 *
 * When the server should be destroyed, it shall disconnect all the clients
 * already connected.
 *
 */
applicationInterface::~applicationInterface()
{
    if(socketList.size()){
        for(int i = 0; i<socketList.size(); i++ ){
            if(socketList[i]->socket != nullptr) {
                socketList[i]->socket->close();
                socketList[i]->socket->deleteLater();
            }
        }
    }
}

void SocketItem::disconnected(void){
    emit itemDisconnected(this->id);
}

void SocketItem::socketError(QAbstractSocket::SocketError error)
{
    emit itemDisconnected(this->id);
    return;
}



/**
 * @brief SocketItem::socketRxData
 *
 * This is the handler of the data reception of a given socket.
 *
 * When a data frame is received, it is decoded and the internal command string is \n
 * forwarded to the Command function handler through the signal 'receivedCommandSgn()'.
 *
 * The function is able to handle queued frames contained in a single data streaming.
 *
 * A valid frame shall be a set of string within the < > delimiters.
 *
 */
void SocketItem::socketRxData()
{
    QByteArray frame;
    QString stringa;

    if(socket->bytesAvailable()==0) return;
    QByteArray data = socket->readAll();

    qDebug() << data;

    bool init_find = true;
    QByteArray streaming = "";

    for(int i=0; i<data.size(); i++){
        if(init_find){
            if(data.at(i) == '<'){
                streaming.clear();
                init_find = false;
            }
        }else{
            if(data.at(i) == '>'){
                if(streaming.size()) emit receivedCommandSgn(this->id, streaming);
                streaming.clear();
                init_find = true;
            }else streaming.append(data.at(i));
        }

    }


}


void SocketItem::socketTxData(QByteArray data)
{
    this->socket->write(data);
    this->socket->waitForBytesWritten(100);
    return;

}


/**
 * @brief Start
 *
 * Starts the Client connection listening
 *
 * @return
 * - True: listening successfully started;
 * - flase: error in activate the listening;
 *
 */
bool applicationInterface::Start(void)
{

    if (!this->listen(localip,localport)){
        qDebug() << "ERROR LISTENING AT ADDRESS: IP=" << localip.toString() << ", PORT=" << localport ;
        return false;
    }else{
        qDebug() << "LISTENING AT ADDRESS: IP=" << localip.toString() << ", PORT=" << localport ;
        return true;
    }

}


void applicationInterface::disconnected(ushort id)
{

    for(int i =0; i < socketList.size(); i++ ){
        if(socketList[i]->id == id){

            disconnect(socketList[i]);
            socketList[i]->socket->deleteLater();
            delete socketList[i];
            socketList.remove(i);
            return;
        }
    }

}

/**
 * @brief applicationInterface::incomingConnection
 * @param socketDescriptor
 *
 * This Slot is connected to the QTcpServer class connection request.
 * Every Client connection is assigned to a proper thread
 * handling the connection.
 *
 * When a client is successfully connected, it will receive the 'EventStatus()'
 * as a Welcome frame.
 *
 */
void applicationInterface::incomingConnection(qintptr socketDescriptor)
{

    // Create a new SocketItem and its internal socket
    // This will be added to the Socket list of the server.
    SocketItem* item = new SocketItem();

    item->socket = new QTcpSocket(this); // Create a new socket
    if(!item->socket->setSocketDescriptor(socketDescriptor))
    {
        delete item->socket;
        delete item;
        return;
    }

    // Add the Client socket to the list of the Connected socket
    item->socket->setSocketOption(QAbstractSocket::LowDelayOption,1);
    socketList.append(item);


    // Interface signal connection
    connect(item->socket,SIGNAL(readyRead()), item, SLOT(socketRxData()),Qt::UniqueConnection);
    connect(item,SIGNAL(receivedCommandSgn(ushort, QByteArray)), this, SLOT(receivedCommandSlot(ushort, QByteArray)),Qt::QueuedConnection);

    connect(item->socket,SIGNAL(disconnected()),item, SLOT(disconnected()),Qt::UniqueConnection);
    connect(this,SIGNAL(txFrame(QByteArray)),item, SLOT(socketTxData(QByteArray)),Qt::QueuedConnection);

    connect(item,SIGNAL(itemDisconnected(ushort )),this, SLOT(disconnected(ushort )),Qt::UniqueConnection);
    connect(item->socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),item,SLOT(socketError(QAbstractSocket::SocketError)),Qt::UniqueConnection);

    // The client is assigned to an unique ID identifier
    item->id = this->idseq++;

    // The Welcome Frame is sent to the client with the current generator status.


    return;
 }



/**
 * @brief sendAck
 *
 * This function format the Acknowledge frame to be sent to the Client
 *
 * The Parameter shall contain only the Acknowledge item,
 * the function will append the proper structure:
 *  > <A SEQ [data] >
 *
 * @param
 * - id: the client identifier that will receive the Ack frame;
 * - seq: sequence number of the received command to be acknowledged;
 * - errcode: defines the error code (if different from 0)
 * - params: the list of optional parameters to be sent to Client.
 *
 */
void applicationInterface::sendAck(ushort id,   ushort seq,  QString command, uint errcode, QList<QString>* params){
    QByteArray buffer;

    // Creates the Acknowledge frame
    buffer.append('<');
    buffer.append('A');
    buffer.append(' ');
    buffer.append(QString("%1").arg(seq).toLatin1());
    buffer.append(' ');
    buffer.append(command.toLatin1());
    buffer.append(' ');
    if(errcode) buffer.append("NOK");
    else buffer.append("OK");
    buffer.append(' ');
    buffer.append(QString("%1").arg(errcode).toLatin1());
    buffer.append(' ');

    if(params){
        // Append the data content of the frame
        for(int i =0; i< params->size(); i++){
                buffer.append(params->at(i).toLatin1());
                buffer.append(' ');
        }
    }

    buffer.append('>');
    qDebug() << buffer;

    // Append a line feed to help a character client to show the frame
    buffer.append('\n');
    buffer.append('\r');

    // Sends only to the socket Id requesting the command
    for(int i=0; i< socketList.size(); i++){
        if(socketList[i]->id == id){
            socketList[i]->socket->write(buffer);
            socketList[i]->socket->waitForBytesWritten(100);
            return;
        }
    }
}

/**
 * @brief sendEvent
 *
 * This function sends an EVENT frame with the format:
 * - <E SEQ Event [params]>
 *
 * The frame is sent broadcast to all the connected Clients.
 *
 * @param
 * - Event: the string identifying the Event code
 * - params: the list of parameter's item of the EVENT.
 *
 */
void applicationInterface::sendEvent(QString Event, QList<QString>* params){
    QByteArray buffer;
    buffer.append('<');
    buffer.append('E');
    buffer.append(' ');
    buffer.append(Event.toLatin1());
    buffer.append(' ');

    // Append the EVENT items
    if(params){
        for(int i =0; i< params->size(); i++){
                buffer.append(params->at(i).toLatin1());
                buffer.append(' ');
        }
    }
    buffer.append('>');
    qDebug() << buffer;

    // Append the line feed to help a character monitor client
    buffer.append('\n');
    buffer.append('\r');

    // Sends broadcast to ALL clients
    for(int i=0; i< socketList.size(); i++){
        socketList[i]->socket->write(buffer);
        socketList[i]->socket->waitForBytesWritten(100);
    }
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
QList<QString> applicationInterface::getProtocolFrame(QByteArray* data){
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


/**
 * @brief receivedCommandSlot
 *
 * This function decodes the data frame providing a list
 * of decoded items that make part of a received EVENT.
 *
 * The Function calls the Subclass handler handleReceivedCommand()
 * to handle the decoded command.
 *
 * The function sends the acknowledge based on the returned code of the
 * handleReceivedCommand().
 *
 * @param
 * - id: the client identifier that sent the EVENT;
 * - data: command data stream.
 *
 */
void applicationInterface::receivedCommandSlot(ushort id, QByteArray data){

    // Extracts the protocol items list
    QList<QString> command =  getProtocolFrame(&data);
    if(command.size() < 3) return;
    if(command.at(0) != "C") return;
    ushort seq = command.at(1).toUShort();

    QList<QString> answer;
    uint errcode = handleReceivedCommand(&command, &answer);
    sendAck(id, seq, command.at(2), errcode, &answer);

}

/**
 * This is the Base Class method  Command handler.
 *
 * The Command handler shall be reimplemented by the Subclass
 * in order to implement the Command interface.
 *
 * The Handler shall return an error code different from zero,
 * in case of an error condition.
 *
 * The Handler shall get the Command Name from frame->at(2)
 * and the optionals parameters from frame->at(3) ... to frame->at(n);
 *
 * The Handler shall populate the Answer list with the optional acknowledge params.
 *
 * @param frame
 *  - frame[2]: is the command string;
 *  - frame[3] to [n]: if presents represents the optional parameters of the command;
 *
 * @param answer
 * - is a list of optonal parameters that the application can send to Client acknowledging a command.
 *
 * @return
 * - is an uint error code:
 *  - 0: no error;
 *  - <>0: error condition;
 */
uint applicationInterface::handleReceivedCommand(QList<QString>* frame, QList<QString>* answer){
    answer->clear(); // Add parameters to the acknowledge if necessary
    return 0; // Return the error code
}
