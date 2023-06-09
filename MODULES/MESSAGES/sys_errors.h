#ifndef SYSERR
#define SYSERR
#include <QApplication>

namespace sysErrors{


    typedef struct{
        ushort  errcode;
        QString errtag;
    }syserr_t;

    #define SYSERR_STARTUP_CAN                          1, QApplication::translate("SYSERR","STARTUP CAN PROCESS")
    #define SYSERR_STARTUP_POWERSERVICE_PROCESS         2, QApplication::translate("SYSERR","STARTUP POWERSERVICE PROCESS")
    #define SYSERR_POWERSERVICE_CANDRIVER_CONNECTION    3, QApplication::translate("SYSERR","POWERSERVICE CAN DRIVER CONNECTION")
    #define SYSERR_POWERSERVICE_PROCESS_REVISION_REQ    4, QApplication::translate("SYSERR","POWERSERVICE REQUEST PROCESS REVISION")
    #define SYSERR_POWERSERVICE_PROCESS_WRONG_REVISION  5, QApplication::translate("SYSERR","POWERSERVICE WRONG PROCESS REVISION")
    #define SYSERR_STARTUP_COMPRESSOR_PROCESS           6, QApplication::translate("SYSERR","STARTUP COMPRESSOR PROCESS")
    #define SYSERR_COMPRESSOR_CANDRIVER_CONNECTION      7, QApplication::translate("SYSERR","COMPRESSOR CAN DRIVER CONNECTION")
    #define SYSERR_COMPRESSOR_PROCESS_REVISION_REQUEST  8, QApplication::translate("SYSERR","COMPRESSOR REQUEST PROCESS REVISION")
    #define SYSERR_COMPRESSOR_PROCESS_WRONG_REVISION    9, QApplication::translate("SYSERR","COMPRESSOR WRONG PROCESS REVISION")

    #define SYSERR_STARTUP_COLLIMATOR_PROCESS           10, QApplication::translate("SYSERR","STARTUP COLLIMATOR PROCESS")
    #define SYSERR_COLLIMATOR_CANDRIVER_CONNECTION      11, QApplication::translate("SYSERR","COLLIMATOR CAN DRIVER CONNECTION")
    #define SYSERR_COLLIMATOR_PROCESS_REVISION_REQUEST  12, QApplication::translate("SYSERR","COLLIMATOR REQUEST PROCESS REVISION")
    #define SYSERR_COLLIMATOR_PROCESS_WRONG_REVISION    13, QApplication::translate("SYSERR","COLLIMATOR WRONG PROCESS REVISION")

    #define SYSERR_STARTUP_FILTER_PROCESS               14, QApplication::translate("SYSERR","STARTUP FILTER PROCESS")
    #define SYSERR_FILTER_CANDRIVER_CONNECTION          15, QApplication::translate("SYSERR","FILTER CAN DRIVER CONNECTION")
    #define SYSERR_FILTER_PROCESS_REVISION_REQUEST      16, QApplication::translate("SYSERR","FILTER REQUEST PROCESS REVISION")
    #define SYSERR_FILTER_PROCESS_WRONG_REVISION        17, QApplication::translate("SYSERR","FILTER WRONG PROCESS REVISION")

    #define SYSERR_STARTUP_POTTER_PROCESS               18, QApplication::translate("SYSERR","STARTUP POTTER PROCESS")
    #define SYSERR_POTTER_CANDRIVER_CONNECTION          19, QApplication::translate("SYSERR","POTTER CAN DRIVER CONNECTION")
    #define SYSERR_POTTER_PROCESS_REVISION_REQUEST      20, QApplication::translate("SYSERR","POTTER REQUEST PROCESS REVISION")
    #define SYSERR_POTTER_PROCESS_WRONG_REVISION        21, QApplication::translate("SYSERR","POTTER WRONG PROCESS REVISION")

    #define SYSERR_STARTUP_GENERATOR_PROCESS            22, QApplication::translate("SYSERR","STARTUP GENERATOR PROCESS")
    #define SYSERR_GENERATOR_CANDRIVER_CONNECTION       23, QApplication::translate("SYSERR","GENERATOR CAN DRIVER CONNECTION")
    #define SYSERR_GENERATOR_PROCESS_REVISION_REQUEST   24, QApplication::translate("SYSERR","GENERATOR REQUEST PROCESS REVISION")
    #define SYSERR_GENERATOR_PROCESS_WRONG_REVISION     25, QApplication::translate("SYSERR","GENERATOR WRONG PROCESS REVISION")

    #define SYSERR_STARTUP_MOTORS_PROCESS               26, QApplication::translate("SYSERR","STARTUP MOTORS PROCESS")
    #define SYSERR_MOTORS_CANDRIVER_CONNECTION          27, QApplication::translate("SYSERR","MOTORS CAN DRIVER CONNECTION")
    #define SYSERR_MOTORS_PROCESS_REVISION_REQUEST      28, QApplication::translate("SYSERR","MOTORS REQUEST PROCESS REVISION")
    #define SYSERR_MOTORS_PROCESS_WRONG_REVISION        29, QApplication::translate("SYSERR","MOTORS WRONG PROCESS REVISION")


    static const syserr_t SYSERR_ARRAY[] =
    {
        {SYSERR_STARTUP_CAN},
        {SYSERR_STARTUP_POWERSERVICE_PROCESS},
        {SYSERR_POWERSERVICE_CANDRIVER_CONNECTION},
        {SYSERR_POWERSERVICE_PROCESS_REVISION_REQ},
        {SYSERR_POWERSERVICE_PROCESS_WRONG_REVISION},

        {SYSERR_STARTUP_COMPRESSOR_PROCESS},
        {SYSERR_COMPRESSOR_CANDRIVER_CONNECTION},
        {SYSERR_COMPRESSOR_PROCESS_REVISION_REQUEST},
        {SYSERR_COMPRESSOR_PROCESS_WRONG_REVISION},

        {SYSERR_STARTUP_COLLIMATOR_PROCESS},
        {SYSERR_COLLIMATOR_CANDRIVER_CONNECTION},
        {SYSERR_COLLIMATOR_PROCESS_REVISION_REQUEST},
        {SYSERR_COLLIMATOR_PROCESS_WRONG_REVISION},

        {SYSERR_STARTUP_FILTER_PROCESS},
        {SYSERR_FILTER_CANDRIVER_CONNECTION},
        {SYSERR_FILTER_PROCESS_REVISION_REQUEST},
        {SYSERR_FILTER_PROCESS_WRONG_REVISION},

        {SYSERR_STARTUP_POTTER_PROCESS},
        {SYSERR_POTTER_CANDRIVER_CONNECTION},
        {SYSERR_POTTER_PROCESS_REVISION_REQUEST},
        {SYSERR_POTTER_PROCESS_WRONG_REVISION},

        {SYSERR_STARTUP_GENERATOR_PROCESS},
        {SYSERR_GENERATOR_CANDRIVER_CONNECTION},
        {SYSERR_GENERATOR_PROCESS_REVISION_REQUEST},
        {SYSERR_GENERATOR_PROCESS_WRONG_REVISION},

        {SYSERR_STARTUP_MOTORS_PROCESS},
        {SYSERR_MOTORS_CANDRIVER_CONNECTION},
        {SYSERR_MOTORS_PROCESS_REVISION_REQUEST},
        {SYSERR_MOTORS_PROCESS_WRONG_REVISION},


    };


}

#endif
