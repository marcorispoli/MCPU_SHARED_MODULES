#ifndef CAN_BOOTLOADER_PROTOCOL_H
#define CAN_BOOTLOADER_PROTOCOL_H



/*!
 * \defgroup  canBootloaderModule CAN Device Bootloader Module Interface.
 * \ingroup libraryModules
 *
 * This library module implements the CAN Bootloader Device interface
 * as described in the document:
 *
 * + Gantry Software Detailed Documentation/Device Protocol Specification
 *
 * # DEPENDENCES
 *
 * This module requires the use of the:
 *
 * + canclient.cpp
 * + canclient.h
 *
 */



#include <QtCore>


/**
 * @brief This class implements the Device Can communication protocol
 *
 * This class should be subclassed from a Parent child class implementing the
 * device communication protocol.
 *
 * The Class:
 * - connects the CAN Driver process;
 * - Register the canID on the CAN Driver;
 * - Handles the can frame reception;
 * - Provides function interfaces to access the Device registers;
 * - Provides the function to execute remote commands;
 *
 */
class canBootloaderProtocol: public QObject
{
    Q_OBJECT

public:

    explicit canBootloaderProtocol(uchar devid, QString ip_driver, uint port_driver);
    ~canBootloaderProtocol();

     static const unsigned short CAN_BOOTLOADER_DEVICE_BASE_ADDRESS = 0x100; //!< This is the Point to Point bootloader device Base address
     static const unsigned short CAN_RXTX_TMO = 100; //!< This defines the maximum reception waiting time in ms

     /**
      *  This enumeration defines the Frame Command Codes
      */
      typedef enum{
          BOOTLOADER_GET_INFO = 1,      //!< Request the bootloader informations
          BOOTLOADER_START,             //!< Request the bootloader to start execution
          BOOTLOADER_EXIT,              //!< Request the bootloader to exit execution
      }CAN_BOOTLOADER_COMMANDS_t;

     /**
      *  This is the Bootloader CAN frame data content
      */
     typedef struct{
         uchar command;      //!< Command code
         uchar d[7];         //!< Command data content
     }CAN_BOOTLOADER_CONTENT_t;

     typedef enum{
         BOOTLOADER_NOT_PRESENT = 0,      //!< The bootloader section is not present in the target appication
         BOOTLOADER_NOT_RUNNING,          //!< The Bootloader is present but not running
         BOOTLOADER_RUNNING,              //!< The bootloader is present and running
     }CAN_BOOTLOADER_PROCSTAT_t;




signals:
    void txToBootloader(ushort canId, QByteArray data); //!< Sends Can data frame to the canDriver
    void dataReceivedFromBootloaderCan(ushort canId, QByteArray data); //!< Emitted when a frame is received for debug purpose

protected:

    bool inline isBootloaderCommunicationPending(void) {return busy;} //!< Test if the last can rx/tx is still pending
    bool inline isBootloaderCommunicationOk(void) {return rxOk;} //!< Test if the last can rx/tx is successfully concluded


    bool inline bootloaderGetInfo(void) {return sendCommand(BOOTLOADER_GET_INFO,0,0,0,0,0,0,0);}
    bool inline bootloaderStart(void) {return sendCommand(BOOTLOADER_START,0,0,0,0,0,0,0);}

    uint8_t inline bootloaderGetCommandResult(uint8_t index){return command_result.at(index);}

    bool inline isBootloaderCommandError(void){return ((uint8_t) command_result.at(0) == 0xFF);}
    uint8_t inline bootloaderGetCommandError(void){return command_result.at(2);}

    bool inline isBootloaderPresent(void){return ((uint8_t) command_result.at(1) != BOOTLOADER_NOT_PRESENT);}
    bool inline isBootloaderRunning(void){return ((uint8_t) command_result.at(1) != BOOTLOADER_RUNNING);}

    uint8_t inline bootloaderGetAppRevMaj(void){return (uint8_t) command_result.at(5);}
    uint8_t inline bootloaderGetAppRevMin(void){return (uint8_t) command_result.at(6);}
    uint8_t inline bootloaderGetAppRevSub(void){return (uint8_t) command_result.at(7);}
    uint8_t inline bootloaderGetBootRevMaj(void){return (uint8_t) command_result.at(2);}
    uint8_t inline bootloaderGetBootRevMin(void){return (uint8_t) command_result.at(3);}
    uint8_t inline bootloaderGetBootRevSub(void){return (uint8_t) command_result.at(4);}



private slots:
   void rxFromBootloader(ushort canId, QByteArray data);//!< Receive Can data frame from the canDriver  
   void bootloaderTmoEvent(void);         //!< Tx/Rx timeout event

protected slots:


private:
    ushort bootloaderID; //!< This is the target device ID


    bool  busy;             //!< Busy flag waiting for the Device answer
    bool  rxOk;             //!< The Frame has been correctly received
    unsigned char req_command;  //!< Last requested command
    QByteArray    command_result;

    struct{
        uchar tmo:1;
        uchar cmd:1;
        uchar crc:1;
        uchar id:1;
        uchar idx:1;
        uchar frame_code:1;
        uchar spare:2;
    }frameError;            //!< In case of error frame, this is the error cause

    QTimer bootloaderTmo;

     bool  sendCommand(canBootloaderProtocol::CAN_BOOTLOADER_COMMANDS_t cmd, uchar d0, uchar d1, uchar d2, uchar d3, uchar d4, uchar d5, uchar d6);
};




#endif // CAN_BOOTLOADER_PROTOCOL_H
