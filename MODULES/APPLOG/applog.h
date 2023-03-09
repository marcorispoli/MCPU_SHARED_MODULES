#ifndef APPLOG_H
#define APPLOG_H

/*!
 * \defgroup  applicationLogModule Application Logger Module Library.
 *
 * This Library Module provides the Logging/Debugging options to the application.
 *
 * If the Application runs with options, the following options are reconnised:
 * - -file: the debug print are redirect to a text file;
 * - -win: the debug prints are redirect to a provided function of an internal Window form (if present)
 * - -console: the debug prints are redirect to the standard console;
 * - No other options: the debug prints are ignored.
 *
 * The Debug Prints are internally generated with the following functions:
 * - qDebug();
 * - qInfo();
 * - qWarning();
 * - qCritical();
 *
 * The -win and the -file can be present at the same time:
 * - in that case the prints are both redirec to file and to the internal window's form.
 *
 * # USAGE
 *
 * In order to activate the Logging, use the constructor in main.cpp file,
 * just after the QApplication instance:
 *
 * \code

    int main(int argc, char *argv[])
    {

        QApplication a(argc, argv);

        // The Application initialize the logger, passing the Application options and (optionally)
        // the pointer to the static function debugWindowClassName::debugMessageHandler if the -win
        // option should be implemented. See the next Section.
        appLog(argc, argv, "path/logfilename", debugWindowClassName::debugMessageHandler);

        ....

        // If the Application option contains the -win option, the flag appLog::isWindow is TRUE
        // an the Application can create the Window Form.
        if(appLog::isWindow) debugWindowClassName* window = new debugWindowClassName();

 *
 * \endcode
 *
 * The appLog::options variable of QString type, will contain all the Application option string.
 *
 * If the application should manage other options than -win, -file and -console,
 * after the use of the appLog() constructor, the Applicaiton can check for other optins with:
 *
 * appLog::options.contain("option_string"). Example:
 *
 * \code
 *  appLog(argc, argv, "path/logfilename", debugWindowClassName::debugMessageHandler);
     ....

     // In this case the extra option to be found is "-loopback"
     bool loopback = false;
 *   if(appLog::options.contain("-loopback")) loopback = true;
 *
 *
 * \endcode
 *
 * # WINDOW FORM USAGE
 *
 * In order to implement a window callback, the Window implementing class shall
 * declare the following static members:
 *
 * \code
 * class debugWindowClassName: ...
 * {
 *      ...
 *
 *  public:
 *      // Declare both the Message handler and pointer to the instance of the window in the application
 *      static void debugMessageHandler(QtMsgType type, QString msg);
 *      inline static debugWindowClassName* instance = nullptr;
 *
 *      ....
 *  }
 *
 * \endcode
 *
 *  In the Class constructor the pointer to the Class instance shall be assigned \n
 *  to the static member debugWindowClassName::instance;
 *
 *  \code
 *
 *  debugWindowClassName::debugWindowClassName{
 *      ...
 *
 *      // Assignes the instance of the Window class to the static member
 *     debugWindowClassName::instance = this;
 *      ... do something
 *  }
 *
 * \endcode
 *
 * In the .cpp file of the Window class, implement the following callback:
 *
 *  \code
 *
 *  void debugWindowClassName::debugMessageHandler(QtMsgType type, QString msg){
 *
 *      if(!debugWindowClassName::instance) return; // Mandatory to prevent any crash!!
 *      ...
 *
 *      // Use any Window method to print the debug message msg
 *      debugWindowClassName::instance-> ...
 *
 *  }
 *
 * \endcode

 *
 * \ingroup libraryModules
 *
 */


#include <QDate>
#include <QFile>

/**
 * @brief The appLog class implementing the Logger class
 *
 * \ingroup applicationLogModule
 */
class appLog
{

public:

    typedef void (*applog_function)(QtMsgType ,QString); //!< Callback function pointer type definition

    appLog(int argc, char *argv[], QString logfile, applog_function win=nullptr); //!< Class Constructor
    ~appLog(){};//!< Class Distructor


    inline static bool     isWindow; //!< True if the option strings contain  -win
    inline static bool     isFile;   //!< True if the option strings contain  -file
    inline static QString  options;  //!< The Application option string

private:
    inline static QFile*   logfd;           //!< Pointer to the log file (if present)
    inline static applog_function winfun;   //!< Pointer to the message callback for Window redirection (if requested)
    static void  messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg); //!< Debug Message Handler installed


};


#endif
