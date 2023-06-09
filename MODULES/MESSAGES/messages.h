#ifndef SYS_MESSAGES
#define SYS_MESSAGES

#include <QApplication>
#include <QDateTime>
#include <QTranslator>
#include "sys_errors.h"


namespace sysMessages{

    Q_GLOBAL_STATIC(QTranslator, pTranslator);

    static void loadLanguage(QString path, QString tag){
        bool ris = false;
        if(tag == "ITA") ris = pTranslator->load("traduzione_ita.qm",path);
        else if(tag == "ENG") ris = pTranslator->load("traduzione_eng.qm",path);
        else if(tag == "FRA") ris = pTranslator->load("traduzione_fra.qm",path);
        else if(tag == "POR") ris = pTranslator->load("traduzione_por.qm",path);
        else if(tag == "ESP") ris = pTranslator->load("traduzione_esp.qm",path);
        else if(tag == "RUS") ris = pTranslator->load("traduzione_rus.qm",path);
        else if(tag == "LIT") ris = pTranslator->load("traduzione_lit.qm",path);
        if (ris)  QApplication::installTranslator(pTranslator);
        else {
            // If the selection should fail the default language is always english
            ris = pTranslator->load("traduzione_eng.qm",path);
            QApplication::installTranslator(pTranslator);
        }
    }
}

namespace sysErrors{

    static inline QString translate(ushort code, QString errstr){ return errstr;}

    static inline QString getErrStr(ushort code){
        for(ushort i=0; i < sizeof(SYSERR_ARRAY)/sizeof(syserr_t); i++){
            if(SYSERR_ARRAY[i].errcode == code) return QApplication::translate("SYSERR",SYSERR_ARRAY[i].errtag.toLocal8Bit());
        }
        return "";
    }

    typedef struct{
        ushort  errcode;
        QString timestamp;
        bool    validated;
    }error_log_t;

    Q_GLOBAL_STATIC(QList<error_log_t>, pErrorLog);

    static void addlog(ushort error, QString errstr){
       QDateTime date_time;
       QString ts = date_time.toString("dd.MM.yyyy.h:m:s ap");
       pErrorLog->append({error, ts, false});
       qDebug() << "< " << ts << " > " << ": " << "ERROR " << error << ", DESCRIPTION: " << errstr;
    };
}



#endif
