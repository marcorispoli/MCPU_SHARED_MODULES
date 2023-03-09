#include "application.h"
#include "can_device_protocol.h"
#include "canclient.h"

static canClient* pCan = nullptr;

/**
 * This is the class constructor.
 *
 * The constructor assignes the deviceID and connect the CAN driver
 *
 * @param devid the target device ID
 * @param ip_driver the IP address of the CAN driver application
 * @param port_driver the port of the CAN Driver application
 *
 */
canDeviceProtocol::canDeviceProtocol(uchar devid, const char* ip_driver, const ushort port_driver)
{
    // Create the Can Client Object to communicate with the can driver process
    deviceID = canDeviceProtocol::CAN_ID_DEVICE_BASE_ADDRESS + devid ;
    pCan = new canClient(0xFFF, deviceID, ip_driver, port_driver);


    // Activation of the communicaitone with the CAN DRIVER SERVER
    connect(pCan, SIGNAL(rxFromCan(ushort , QByteArray )), this, SLOT(rxFromCan(ushort , QByteArray )), Qt::QueuedConnection);
    connect(this,SIGNAL(txToCan(ushort , QByteArray )), pCan,SLOT(txToCanData(ushort , QByteArray )), Qt::QueuedConnection);
    pCan->ConnectToCanServer();

    frame_sequence = 1;
    busy = false;
    tmoTimer = 0;
}

canDeviceProtocol::~canDeviceProtocol()
{

}

/**
 * @brief This function returns the current detected frame error
 * @return the frame error readable string
 */
QString canDeviceProtocol::getFrameErrorStr(void){
    if(frameError.tmo) return "TIMEOUT FRAME";
    if(frameError.seq) return "SEQ ERR FRAME";
    if(frameError.crc) return "CRC ERR FRAME";
    if(frameError.id) return "ID ERR FRAME";
    if(frameError.idx) return "IDX ERR FRAME";
    if(frameError.frame_code) return "FRAME CODE ERR FRAME";
    return "";
}

/**
 * @brief This is the CAN frame reception handler
 *
 * The function is called when a can frame is received.
 *
 * The function decode the content and allow to proceed with the
 * protocol implementation only if the following rules are meet:
 * + there isn't a pending reception frame;
 * + the received device ID matches with the internal device ID;
 * + the sequence number matches with the expected;
 *
 *
 * @param devId received device ID
 * @param data CAN data frame to be processed
 */
void canDeviceProtocol::rxFromCan(ushort devId, QByteArray data){
    emit dataReceivedFromCan(devId,data); // For debug

    // No pending reception
    if(!busy) return;

    // Stops the timeout
    if(tmoTimer) killTimer(tmoTimer);
    tmoTimer = 0;

    // Device ID not matching the expected
    if(devId != deviceID) {
        busy = false;
        rxOk = false;
        frameError.id = 1;
        return; // Invalid ID
    }


    // Test the received sequence number mutches the sent frame
    canDeviceProtocolFrame::CAN_FRAME_CONTENT_t content = canDeviceProtocolFrame::toContent(&data);
    if(content.seq == 0){
        busy = false;
        rxOk = false;
        frameError.seq = 1;
        return; // Invalid Frame Format (lenght or crc)
    }

    if(frame_sequence != content.seq){
        busy = false;
        rxOk = false;
        frameError.seq = 1;        
        return;
    }

    // Received a correct protocol frame    
    receptionEvent(&content);
    return;
}

/**
 * This is the timer event routine used to detect a transmission timeout
 *
 * @param event
 */
void canDeviceProtocol::timerEvent(QTimerEvent *event){
    if(event->timerId() == tmoTimer){
        killTimer(tmoTimer);
        tmoTimer = 0;
        if(!busy) return;

        // Timeout Event
        qDebug() << "TIMEOUT EVENT";

        busy = false;
        rxOk = false;
        frameError.tmo = 1;
        return;
    }
}



/**
 * @brief This is the interface function to request a protocol register access.
 *
 *
 * @param regtype is the type of access
 * @param idx is the register or command idx code
 * @param d0 frame data 0
 * @param d1 frame data 1
 * @param d2 frame data 2
 * @param d3 frame data 3
 *
 * @return true if the frame is correctly sent to the CAN bus
 */
bool  canDeviceProtocol::accessRegister(canDeviceProtocolFrame::CAN_FRAME_COMMANDS_t regtype, uchar idx, uchar d0, uchar d1, uchar d2, uchar d3){
    if(!pCan) return false;
    if(busy) return false;
    canDeviceProtocolFrame::CAN_FRAME_CONTENT_t content;

    // Assignes a new frame sequence
    frame_sequence++;
    if(!frame_sequence) frame_sequence = 1;

    // Fills the register content
    content.seq = frame_sequence;
    content.frame_type = regtype;
    content.idx = idx;
    content.d[0] = d0;
    content.d[1] = d1;
    content.d[2] = d2;
    content.d[3] = d3;

    // Initialize the frame status variables
    busy = true;
    rxOk = false;
    (*(uchar*) &frameError) = 0;


    emit txToCan(deviceID, canDeviceProtocolFrame::toCanData(&content));
    if(tmoTimer) killTimer(tmoTimer);
    tmoTimer = startTimer(20);
    return true;
}




/**
 * This function is internally called by the Module when a can frame has been successfully decoded.
 *
 * @param pContent this is the pointer to the decoded protocol frame
 */
void canDeviceProtocol::receptionEvent( canDeviceProtocolFrame::CAN_FRAME_CONTENT_t* pContent){

    // Evaluate the operation
    switch(pContent->frame_type){
        case canDeviceProtocolFrame::READ_REVISION:
            memcpy(revisionRegister.d, pContent->d, 4);
            revisionRegister.valid = true;
            break;
        case canDeviceProtocolFrame::READ_ERRORS:
            memcpy(errorsRegister.d, pContent->d, 4);
            errorsRegister.valid = true;
            break;

        case canDeviceProtocolFrame::COMMAND_EXEC:
        case canDeviceProtocolFrame::READ_COMMAND:
            commandRegister.command = pContent->idx;
            commandRegister.status =  pContent->d[0];
            commandRegister.b0 =      pContent->d[1];
            commandRegister.b1 =      pContent->d[2];
            commandRegister.error =   pContent->d[3];
            commandRegister.valid = true;
            break;
        case canDeviceProtocolFrame::READ_STATUS:
            if(pContent->idx >= statusRegisters.size() ){
                busy = false;
                rxOk = false;
                frameError.idx = 1;
                return;
            }

            memcpy(statusRegisters[pContent->idx].d, pContent->d,4);
            statusRegisters[pContent->idx].valid = true;
            break;
        case canDeviceProtocolFrame::READ_DATA:
            if(pContent->idx >= dataRegisters.size() ){
                busy = false;
                rxOk = false;
                frameError.idx = 1;
                return;
            }

            memcpy(dataRegisters[pContent->idx].d, pContent->d,4);
            dataRegisters[pContent->idx].valid = true;
            break;
        case canDeviceProtocolFrame::READ_PARAM:
            if(pContent->idx >= paramRegisters.size() ){
                busy = false;
                rxOk = false;
                frameError.idx = 1;
                return;
            }

            memcpy(paramRegisters[pContent->idx].d, pContent->d,4);
            paramRegisters[pContent->idx].valid = true;
            break;
        case canDeviceProtocolFrame::WRITE_DATA:
            if(pContent->idx >= dataRegisters.size() ){
                busy = false;
                rxOk = false;
                frameError.idx = 1;
                return;
            }

            memcpy(dataRegisters[pContent->idx].d, pContent->d,4);
            dataRegisters[pContent->idx].valid = true;
            break;
        case canDeviceProtocolFrame::WRITE_PARAM:
            if(pContent->idx >= paramRegisters.size() ){
                busy = false;
                rxOk = false;
                frameError.idx = 1;
                return;
            }

            memcpy(paramRegisters[pContent->idx].d, pContent->d,4);
            paramRegisters[pContent->idx].valid = true;
            break;

        case canDeviceProtocolFrame::STORE_PARAMS:
            break;


        default:

            busy = false;
            rxOk = false;
            frameError.frame_code = 1;
            return;
    }

    // Reception completed
    busy = false;
    rxOk = true;
    return;
}


