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
    #define SYS_LOCALHOST_PARAM         "LOCALHOST"
    #define SYS_AWSPORT_PARAM           "AWS_PORT"
    #define SYS_GENERATOR_PORT_PARAM    "GENERATOR_PORT"
    #define SYS_POWERSERVICE_PORT_PARAM "POWER_SERVICE_PORT"
    #define SYS_FILTER_PORT_PARAM       "FILTER_PORT"
    #define SYS_COLLIMATOR_PORT_PARAM   "COLLIMATOR_PORT"
    #define SYS_COMPRESSOR_PORT_PARAM   "COMPRESSOR_PORT"
    #define SYS_POTTER_PORT_PARAM       "POTTER_PORT"
    #define SYS_MOTORS_PORT_PARAM       "MOTORS_PORT"
    #define SYS_BIOPSY_PORT_PARAM       "BIOPSY_PORT"

    // your class constructor
    sysConfig():configFile( (const configFile::fileDescriptorT)
        {
            SYS_CONFIG_FILENAME, SYS_REVISION,
            {{
                { SYS_LOCALHOST_PARAM,          {{ "127.0.0.1" }},   "Address of the localhost used for application"},
                { SYS_AWSPORT_PARAM,            {{ "10000" }},       "Port for the AWS connection"},
                { SYS_GENERATOR_PORT_PARAM,     {{ "10001" }},       "Port for generator driver"},
                { SYS_POWERSERVICE_PORT_PARAM,  {{ "10002" }},       "Port for Power service driver"},
                { SYS_FILTER_PORT_PARAM,        {{ "10003" }},       "Port for Filter driver"},
                { SYS_COLLIMATOR_PORT_PARAM,    {{ "10004" }},       "Port for Collimator driver"},
                { SYS_COMPRESSOR_PORT_PARAM,    {{ "10005" }},       "Port for Compressor driver"},
                { SYS_POTTER_PORT_PARAM,        {{ "10006" }},       "Port for Potter driver"},
                { SYS_MOTORS_PORT_PARAM,        {{ "10009" }},       "Port for Motors driver"},
                { SYS_BIOPSY_PORT_PARAM,        {{ "10010" }},       "Port for Biopsy driver"},

            }}
        })
    {
        // Your constructor code ...
        this->loadFile();
    };


}; // End class definition

#endif // CONFIG_H
