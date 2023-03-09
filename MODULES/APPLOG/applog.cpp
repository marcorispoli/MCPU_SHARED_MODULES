#include "applog.h"

/**
 * This is the constructor of the class.
 *
 * The Constructor decodes the Aplication options
 * and install properly the hanler of the debug messages.
 *
 * The Application option strings is than provided as a public
 * static member QString appLog::options for external usages.
 *
 *
 * @param:
 * - argc: num of options of the application;
 * - argv[]: pointer to the optoion strings;
 * - logfile: path and filename of the log file (if requested);
 * - win: pointer to a static function of the Window Form handling the debug messages;
 *
 */
appLog::appLog(int argc, char *argv[], QString logfile, applog_function win){

    // Inizializes all the internal flags
    appLog::isWindow = false;
    appLog::isFile = false;
    appLog::winfun = win;

    // Translate the Applicaion options from the char* vector to a single string with space separators
    for(int i=0; i<argc; i++){
        options += QString(argv[i]) + " ";
    }

    // Revert the backslash characters
    options.replace(QString("\\"),QString("/"));
    options.replace("//","/");

    // If The option is -console, the other options are ignored
    if(options.contains("-console")){
        qInstallMessageHandler(0);
        return;
    }

    // If the option -file is detected, the file is open.
    if(options.contains("-file")) {
        appLog::logfd = new QFile(logfile);
        if (appLog::logfd->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)){
            appLog::isFile = true;
        }
    }

    // If the option -win is detected, the related flag is set to true
    if(options.contains("-win")) {
        if(appLog::winfun) appLog::isWindow = true;
    }


    // The message handler is installed in the application
    qInstallMessageHandler(messageHandler);

    // The First string logged is the Full option string with the current date
    QDate date;
    date.currentDate().toString();
    qDebug() << date.currentDate().toString() << "APPLICATION STARTED: " << options;
}



/**
 *  This is the message handler callback handling the strings
 *  of the qDebug() qInfo() qWarning() and qCritical() functions.
 *
 *  The message string is internally formatted, prepending
 *  the current clock in seconds and the TAG identifying the
 *  type of debug string.
 *
 *  The Final message string is formatted as follows:
 *
 *  seconds.milliseconds> TAG: message string
 *
 *  Where TAG:
 *  - DBG: for qDebug() messages;
 *  - INFO: for qInfo() messages;
 *  - WARN: for qWarning() messages;
 *  - CRITICAL: for qCritical() messages;
 *  - FATAL: for qFatal() messages.
 *
 *
 *
 * @param type This is the type of the debug message
 * @param context
 * @param msg this is the string of the message
 *
 */
void appLog::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if((!isWindow) && (!isFile)) return;

    QByteArray localMsg;

    switch (type) {
    case QtDebugMsg:

        localMsg = QString("%1> DBG: %2").arg(((double) clock())/CLOCKS_PER_SEC).arg(msg).toLocal8Bit();
        break;

    case QtInfoMsg:
        localMsg = QString("%1> INFO: %2").arg(((double) clock())/CLOCKS_PER_SEC).arg(msg).toLocal8Bit();
        break;

    case QtWarningMsg:
        localMsg = QString("%1> WARN: %2").arg(((double) clock())/CLOCKS_PER_SEC).arg(msg).toLocal8Bit();
        break;

    case QtCriticalMsg:
        localMsg = QString("%1> CRITICAL: %2").arg(((double) clock())/CLOCKS_PER_SEC).arg(msg).toLocal8Bit();
        break;

    case QtFatalMsg:
        localMsg = QString("%1> FATAL: %2").arg(((double) clock())/CLOCKS_PER_SEC).arg(msg).toLocal8Bit();
        break;
    }

    if(isWindow) winfun(type, localMsg);


    if(isFile){
        // The message is appended to the log file
        logfd->write(localMsg.data());
        logfd->write("\n");
        logfd->flush();
    }

}
