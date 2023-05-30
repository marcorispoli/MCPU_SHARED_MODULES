#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#include <QObject>
#include "configfile.h"

class sysConfig : public configFile
{
    public:


    #define SYS_REVISION     1  // This is the revision code
    #define SYS_CONFIG_FILENAME     "C:\\OEM\\Gantry\\Config\\SysConfig.cnf" // This is the configuration file name and path

    // This section defines labels helping the param identification along the application
    #define SYS_AWSPORT_PARAM              "AWS_PARAMS"
    #define SYS_CAN_PROCESS_PARAM          "CAN_PROCESS"
    #define SYS_GENERATOR_PROCESS_PARAM    "GENERATOR_PROCESS"
    #define SYS_POWERSERVICE_PROCESS_PARAM "POWER_SERVICE_PROCESS"
    #define SYS_FILTER_PROCESS_PARAM       "FILTER_PROCESS"
    #define SYS_COLLIMATOR_PROCESS_PARAM   "COLLIMATOR_PROCESS"
    #define SYS_COMPRESSOR_PROCESS_PARAM   "COMPRESSOR_PROCESS"
    #define SYS_POTTER_PROCESS_PARAM       "POTTER_PROCESS"
    #define SYS_MOTORS_PROCESS_PARAM       "MOTORS_PROCESS"
    #define SYS_BIOPSY_PROCESS_PARAM       "BIOPSY_PROCESS"

    #define SYS_AWS_IP          0
    #define SYS_AWS_PORT        1

    #define SYS_CAN_IP          2
    #define SYS_CAN_PORT        3
    #define SYS_CAN_SERVICE     4

    #define SYS_PROCESS_NAME    0
    #define SYS_PROCESS_PARAM   1
    #define SYS_PROCESS_IP      2
    #define SYS_PROCESS_PORT    3


    // your class constructor
    sysConfig():configFile( (const configFile::fileDescriptorT)
        {
            SYS_CONFIG_FILENAME, SYS_REVISION,
            {{
                { SYS_AWSPORT_PARAM,               {{ "127.0.0.1", "10000" }},       "AWS Connection IP and Port"},
                { SYS_CAN_PROCESS_PARAM,           {{ "C:\\OEM\\Gantry\\bin\\CanDriver.exe", "-file" , "127.0.0.1", "10001", "10002"} },  "CAN Driver process"},
                { SYS_GENERATOR_PROCESS_PARAM,     {{ "C:\\OEM\\Gantry\\bin\\Generator.exe", "-file" , "127.0.0.1", "10003" } },  "Generator Driver process"},
                { SYS_POWERSERVICE_PROCESS_PARAM,  {{ "C:\\OEM\\Gantry\\bin\\PowerService.exe", "-file" , "127.0.0.1", "10004" } },  "Power Service Driver process"},
                { SYS_FILTER_PROCESS_PARAM,        {{ "C:\\OEM\\Gantry\\bin\\Filter.exe", "-file" , "127.0.0.1", "10005" } },  "Filter Driver process"},
                { SYS_COLLIMATOR_PROCESS_PARAM,    {{ "C:\\OEM\\Gantry\\bin\\Collimator.exe", "-file" , "127.0.0.1", "10006" } },  "Collimator Driver process"},
                { SYS_COMPRESSOR_PROCESS_PARAM,    {{ "C:\\OEM\\Gantry\\bin\\Compressor.exe", "-file" , "127.0.0.1", "10007" } },  "Compressor Driver process"},
                { SYS_POTTER_PROCESS_PARAM,        {{ "C:\\OEM\\Gantry\\bin\\Potter.exe", "-file" , "127.0.0.1", "10008" } },  "Potter Driver process"},
                { SYS_MOTORS_PROCESS_PARAM,        {{ "C:\\OEM\\Gantry\\bin\\Motors.exe", "-file" , "127.0.0.1", "10009" } },  "Motors Driver process"},
                { SYS_BIOPSY_PROCESS_PARAM,        {{ "C:\\OEM\\Gantry\\bin\\Biopsy.exe", "-file" , "127.0.0.1", "10010" } },  "Biopsy Driver process"},

            }}
        })
    {
        // Your constructor code ...
        this->loadFile();
    };


}; // End class definition

#endif // CONFIG_H
