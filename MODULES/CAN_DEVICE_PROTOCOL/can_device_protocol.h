#ifndef CAN_DEVICE_PROTOCOL_H
#define CAN_DEVICE_PROTOCOL_H



/*!
 * \defgroup  canDeviceModule CAN Device Protocol Module Interface.
 * \ingroup libraryModules
 *
 * This library module implements the CAN Device Protocol interface
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
#include "can_bootloader_protocol.h"

/**
 * @brief The canDeviceProtocolFrame class
 *
 * This class implements the protocol data types and access
 */
class canDeviceProtocolFrame
{
public:

    static const uchar CAN_ABORT_COMMAND = 0; //!< Defines the special command code for abort procedure

   /**
    *  This enumeration defines the Frame Command Codes
    */
    typedef enum{
        READ_REVISION = 1,      //!< Request to read the Revision register
        READ_ERRORS,            //!< Request to read the Errors register
        READ_COMMAND,           //!< Request to read the Command register
        READ_STATUS,            //!< Request to read a Status register
        READ_DATA,              //!< Request to read a Data register
        READ_PARAM,             //!< Request to read a Parameter register
        WRITE_DATA,             //!< Request to Write a Data register
        WRITE_PARAM,            //!< Request to Write a Parameter register
        STORE_PARAMS,           //!< Request to Store in non volatile memory the parameters
        COMMAND_EXEC,           //!< Command Execution	Request
    }CAN_FRAME_COMMANDS_t;

    /**
     *  This is the enumeration of the COMMAND register status field
     */
    typedef enum{
        CAN_COMMAND_EXECUTING=1,         //!< The Command is processing
        CAN_COMMAND_EXECUTED,            //!< The Command has been processed successfully
        CAN_COMMAND_ERROR,               //!< The Command has been terminated with error condition
        CAN_COMMAND_STATUS_UNASSIGNED,   //!< The Application forgot to assign a correct return code

    }CAN_CommandExecStatus_t;

    /**
     *  This is the Command Execution code error enumeraiton list
     */
    typedef enum{
        CAN_COMMAND_NO_ERROR = 0,       //!< No Error
        CAN_COMMAND_BUSY = 1,           //!< A command is executing
        CAN_COMMAND_INVALID_DATA = 2,   //!< The Command data are invalid
        CAN_COMMAND_NOT_ENABLED = 3,    //!< The Command cannot be momentary executed
        CAN_COMMAND_NOT_AVAILABLE = 4,  //!< The Command cannot be executed in this revision
        CAN_COMMAND_WRONG_RETURN_CODE = 5,  //!< The Command returned a non valid status code
        CAN_COMMAND_ABORT_CODE = 6,     //!< The Command has been aborted
        CAN_COMMAND_APPLICATION_ERRORS  //!< Starting code for applicaiton specific errors
    }CAN_CommandErrors_t;

    /**
     *  This is the CAN frame data content excluded the CRC code
     */
    typedef struct{
        uchar seq;          //!< Message sequence code
        uchar frame_type;   //!< Message frame code
        uchar idx;          //!< Message IDX code (register address or command code)
        uchar d[4];         //!  Frame data (Register content or Command parameters)
    }CAN_FRAME_CONTENT_t;

    /**
     *  This is the Register data content
     */
    typedef struct{
        uchar d[4]; //!< Data content
        bool valid; //!< Data Validity
    }CAN_REGISTER_t;

    /**
     *  This is the COMMAND register data content
     */
    typedef struct{        
        uchar command;  //!< Executing Command code
        uchar status;   //!< Executing command status
        uchar b0;       //!< Executed result byte 0
        uchar b1;       //!< Executed result byte 1
        uchar error;    //!< Executed error condition
        bool valid;     //!< Data Validity
    }CAN_COMMAND_t;


    /**
     * This statis method converts the CAN data frame into a protocol decoded frame
     *
     * @param data pointer to the CAN frame
     *
     * @return the decoded protocol data frame
     * if the returned seq field should 0 then the
     * can frame shal be discarded due to wrong crc or length.
     *
     */
    static CAN_FRAME_CONTENT_t toContent(QByteArray* data){
        CAN_FRAME_CONTENT_t content;
        content.seq = 0;

        uchar crc = 0;
        if(data->size() != 8) return content;

        for(int i=0; i<7; i++) crc ^= (uchar) data->at(i);
        if(crc != (uchar) data->at(7)) return content;

        content.seq = data->at(0);
        content.frame_type = (CAN_FRAME_COMMANDS_t) data->at(1);
        content.idx = data->at(2);
        for(int i=0; i< 4; i++) content.d[i] = data->at(3+i);
        return content;

    }

    /**
     * @brief This static function encode a CAN frame with the data of the protocol frame
     *
     * @param content this is the protocol frame
     * @return the data to be sent on the CAN bus
     *
     */
    static QByteArray toCanData(CAN_FRAME_CONTENT_t* content){
        QByteArray data;
        data.append(content->seq);
        data.append(content->frame_type);
        data.append(content->idx);
        data.append(content->d[0]);
        data.append(content->d[1]);
        data.append(content->d[2]);
        data.append(content->d[3]);
        uchar crc = 0;
        for(int i=0; i<7; i++) crc ^= data.at(i);
        data.append(crc);
        return data;
    }

    /**
     * @brief This static function provides a readable string related to a COMMAND execution error code
     *
     * @param code this is the error code
     * @return the string related to the error code
     */
    static QString getCommandErrorStr(uint8_t code){

        switch(code){
        case CAN_COMMAND_NO_ERROR: return "NO ERRORS";
        case CAN_COMMAND_BUSY: return "COMMAND BUSY";
        case CAN_COMMAND_INVALID_DATA: return "COMMAND INVALID DATA";
        case CAN_COMMAND_NOT_ENABLED: return "COMMAND NOT ENABLED";
        case CAN_COMMAND_NOT_AVAILABLE: return "COMMAND NOT AVAILABLE";
        case CAN_COMMAND_WRONG_RETURN_CODE: return "COMMAND WRONG STATUS CODE";
        case CAN_COMMAND_ABORT_CODE: return "COMMAND ABORTED";
        case CAN_COMMAND_APPLICATION_ERRORS:
        default:
            return "APPLICATION ERROR CODE RANGE";
        }
        return "";
    }
};

/**
 * @brief This class implements the Device Can communication protocol
 *
 * This class should be subclassed from a Parent child class implementing the
 * device communication protocol.
 *
 * The Class:
 * - connects the CAN Driver process;
 * - Register the canID on the CANB Driver;
 * - Handles the can frame reception;
 * - Provides function interfaces to access the Device registers;
 * - Provides the function to execute remote commands;
 *
 */
class canDeviceProtocol: public canBootloaderProtocol
{
    Q_OBJECT

public:

    explicit canDeviceProtocol(uchar devid, QString ip_driver, const ushort port_driver);
    ~canDeviceProtocol();

     static const unsigned short CAN_PROTOCOL_DEVICE_BASE_ADDRESS = 0x200; //!< This is the Point to Point protocol device Base address
     static const unsigned short CAN_RXTX_TMO = 100; //!< This defines the maximum reception waiting time in ms

signals:
    void txToDeviceCan(ushort canId, QByteArray data); //!< Sends Can data frame to the canDriver
    void dataReceivedFromDeviceCan(ushort canId, QByteArray data); //!< Emitted when a frame is received for debug purpose

protected:
    bool  deviceAccessRegister(canDeviceProtocolFrame::CAN_FRAME_COMMANDS_t regtype, uchar idx=0, uchar d0=0, uchar d1=0, uchar d2=0, uchar d3=0);
    bool  inline deviceAbortCommand(void){return deviceAccessRegister(canDeviceProtocolFrame::COMMAND_EXEC, canDeviceProtocolFrame::CAN_ABORT_COMMAND);};
    QString getDeviceFrameErrorStr(void);

    canDeviceProtocolFrame::CAN_REGISTER_t           deviceRevisionRegister; //!< Protocol special revision register
    canDeviceProtocolFrame::CAN_REGISTER_t           deviceErrorsRegister;   //!< Protocol special errors register
    canDeviceProtocolFrame::CAN_COMMAND_t            deviceCommandRegister;  //!< Protocol special command register
    QList<canDeviceProtocolFrame::CAN_REGISTER_t>    deviceStatusRegisters;  //!< Array of the device STATUS register
    QList<canDeviceProtocolFrame::CAN_REGISTER_t>    deviceDataRegisters;    //!< Array of the device DATA register
    QList<canDeviceProtocolFrame::CAN_REGISTER_t>    deviceParamRegisters;   //!< Array of the device PARAMETER register

    bool inline isDeviceCommunicationPending(void) {return busy;} //!< Test if the last can rx/tx is still pending
    bool inline isDeviceCommunicationOk(void) {return rxOk;} //!< Test if the last can rx/tx is successfully concluded
    bool inline isCanConnected(void) {return canDriverConnected;} //!< Test if the CAN server process is connected

private slots:
   void rxFromDeviceCan(ushort canId, QByteArray data);//!< Receive Can data frame from the canDriver
   void deviceTmoEvent(void);         //!< Timer event used for the rx/tx timeout
   void canDriverConnectionStatus(bool status){canDriverConnected = status; }



private:
    bool canDriverConnected; //!< THis is the current connection status with the CAN driver process
    ushort deviceID; //!< This is the target device ID
    void receptionEvent(canDeviceProtocolFrame::CAN_FRAME_CONTENT_t* pContent); //!< Function handling a received frame
    uchar frame_sequence;   //!< Frame sequence iterator
    bool  busy;             //!< Busy flag waiting for the Device answer
    bool  rxOk;             //!< The Frame has been correctly received

    struct{
        uchar tmo:1;
        uchar seq:1;
        uchar crc:1;
        uchar id:1;
        uchar idx:1;
        uchar frame_code:1;
        uchar spare:2;
    }frameError;            //!< In case of error frame, this is the error cause

     QTimer deviceTmo;

};




#endif // CAN_DEVICE_PROTOCOL_H
