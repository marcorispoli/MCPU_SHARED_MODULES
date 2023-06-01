#include "application.h"
#include "can_bootloader_protocol.h"
#include "canclient.h"


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
canBootloaderProtocol::canBootloaderProtocol(uchar devid, QString ip_driver, uint port_driver)
{
    // Create the Can Client Object to communicate with the can driver process
    bootloaderID = canBootloaderProtocol::CAN_BOOTLOADER_DEVICE_BASE_ADDRESS + devid ;

    // Activation of the communicaitone with the CAN DRIVER SERVER
    canClient* pBoot = new canClient(0xFFF, bootloaderID, ip_driver, port_driver);
    connect(pBoot, SIGNAL(rxFromCan(ushort , QByteArray )), this, SLOT(rxFromBootloader(ushort , QByteArray )), Qt::QueuedConnection);
    connect(this,SIGNAL(txToBootloader(ushort , QByteArray )), pBoot,SLOT(txToCanData(ushort , QByteArray )), Qt::QueuedConnection);
    pBoot->ConnectToCanServer();

    req_command = 0;
    busy = false;
    connect(&bootloaderTmo, SIGNAL(timeout()), this, SLOT(bootloaderTmoEvent()), Qt::UniqueConnection);
    board_initialized = false;

}

canBootloaderProtocol::~canBootloaderProtocol()
{

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
void canBootloaderProtocol::rxFromBootloader(ushort devId, QByteArray data){
    emit dataReceivedFromBootloaderCan(devId,data); // For debug

    // No pending reception
    if(!busy) return;

    // Stops the timeout
    bootloaderTmo.stop();

    // Timeout signaled by the client
    if(devId==0){
        busy = false;
        rxOk = false;
        frameError.tmo = 1;
        return;
    }

    // Device ID not matching the expected
    if(devId != bootloaderID) {
        busy = false;
        rxOk = false;
        frameError.id = 1;
        return; // Invalid ID
    }

    if(data.length() !=8 ){
        busy = false;
        rxOk = false;
        frameError.cmd = 1;
        return;
    }

    if(((unsigned char) data.at(0) == 0xFF) && ((unsigned char) data.at(1) != req_command)){
        busy = false;
        rxOk = false;
        frameError.cmd = 1;
        return;
    }

    if(((unsigned char) data.at(0) != 0xFF) && ((unsigned char) data.at(0) != req_command)){
        busy = false;
        rxOk = false;
        frameError.cmd = 1;
        return;
    }

    command_result = data;

    if((uchar) data.at(0) == 0xFF){
        bootloader_rxcommand =  data.at(1);
        bootloader_error  =  data.at(2);
    }else if((uchar) data.at(0) == BOOTLOADER_GET_INFO){
        bootloader_rxcommand =  data.at(0);
        bootloader_error = 0;
        bootloader_present = ((uint8_t) data.at(1) != BOOTLOADER_NOT_PRESENT);
        bootloader_running = ((uint8_t) data.at(1) == BOOTLOADER_RUNNING);
        bootloaderMaj = (uint8_t) data.at(2);
        bootloaderMin = (uint8_t) data.at(3);
        bootloaderSub = (uint8_t) data.at(4);
        boardAppMaj = (uint8_t) data.at(5);
        boardAppMin = (uint8_t) data.at(6);
        boardAppSub = (uint8_t) data.at(7);
    }


    busy = false;
    rxOk = true;
    return;

}

/**
 * This is the timer event routine used to detect a transmission timeout
 *
 * @param event
 */
void canBootloaderProtocol::bootloaderTmoEvent(void){
    bootloaderTmo.stop();
    if(!busy) return;

    // Timeout Event
    qDebug() << "BOOTLOADER TIMEOUT EVENT";

    busy = false;
    rxOk = false;
    frameError.tmo = 1;
    return;
}



bool  canBootloaderProtocol::sendCommand(canBootloaderProtocol::CAN_BOOTLOADER_COMMANDS_t cmd, uchar d0, uchar d1, uchar d2, uchar d3, uchar d4, uchar d5, uchar d6){

    if(busy) return false;


    QByteArray frame;

    frame.append((unsigned char) cmd);
    frame.append(d0);
    frame.append(d1);
    frame.append(d2);
    frame.append(d3);
    frame.append(d4);
    frame.append(d5);
    frame.append(d6);

    // Initialize the frame status variables
    busy = true;
    rxOk = false;
    req_command = cmd;

    emit txToBootloader( bootloaderID, frame);
    bootloaderTmo.start(5000);
    return true;
}


