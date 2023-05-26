#ifndef WINDOWS_H
#define WINDOWS_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QApplication>
#include <QScreen>

/**
 * \defgroup LIB_MODULES Library modules
 *
 * This modules group document the library modules of the application.
 */

//! This is the main Class to be subclassed for Windows framework
/*!
 *
 * \addtogroup WIN_MODULE Windows module
 *
 * # ABSTRACT
 * This class inherits the QWidget feature and so can be subclassed in order to
 * create a window GUI integrated with the program window system.
 *
 * The Window system allows to keep the windows connected
 * and properly syncronized preventing the developer to handle
 * the apparence order, and inizialization problems.
 *
 * The windowsClass library main feature are:
 * + open and close windows activities;
 * + direct open or parent open feature;
 * + open and close action are asyncronous;
 * + Window view rotation tools;
 * + Target screen selection in a multi screen system;
 *
 * # USAGE
 * Follows a guide to correct implementing a windowsClass based GUI.
 *
 * ## UI Based subclass Window
 * This section guide to develop a windowsClass based GUI using the QDesigner
 * of the Qt Creator.
 * 1) Create the GUI with the QDesigner:
 * + The gui shall have a main Frame that old all the other objects;
 * + The main frame shall have the dimension of the rotating part of the screen,
 * usually it should be a squared frame with the dimension of the smaller part of the screen.
 * 2) Create the declaration class of the UI based class as follows:
 * \verbatim
 *  #include <QWidget>
 *  #include "windows.h"
 *  QT_BEGIN_NAMESPACE
 *  namespace Ui { class myUiWindow; }
 *  QT_END_NAMESPACE
 *
 *  class myUiWindow : public windowsClass
 *  {
 *      Q_OBJECT
 *
 *  public:
 *      // W is the target screen width, H is the target screen height
 *      myUiWindow(int W, int H, .. other ... ,QWidget *parent = nullptr);
 *      ~myUiWindow();
 *
 *  protected:
 *      virtual void initWindow(void); //!< Override function for the opening activities
 *      virtual void exitWindow(void); //!< Override function for the exiting activities
 *
 *  private:
 *     Ui::myUiWindow *ui; // this is the name of the QWidget object into the UI interface
 *  };
 * \endverbatim
 *
 * 3) Create the CPP class with the following template:
 * \verbatim
 *  #include "myUiWindow.h"
 *  #include "ui_myUiWindow.h"
 *
 *  myUiWindow::myUiWindow(int W, int H, ..other here... , QWidget *parent)
 *     : windowsClass(W, H, parent), ui(new Ui::myUiWindow)
 *  {
 *     ui->setupUi(this);
 *
 *     // Write here your constructor code ...
 *  }
 *
 *  myUiWindow::~myUiWindow()
 *  {
 *    delete ui;
 *  }
 *
 *  void myUiWindow::initWindow(void)
 *  {
 *     Write here your initialization code ..
 *     This callback is called whenever the window open
 *
 *  }
 *
 *  void myUiWindow::exitWindow(void)
 *  {
 *     Write here your exit window code ..
 *     This callback is called whenever the window closes
 *
 *  }
 *
 *
 *\endverbatim
 *
 * \ingroup LIB_MODULES
 *
 */
class windowsClass : public QWidget
{
    Q_OBJECT

public:

    /**
     * @brief windowsClass constructor
     *
     * The constructor uses the Width and Height pixel screen
     * parameter in order to point to the correct target screen.
     *
     * In case the W and H don't match with an available screen, then the first available screen is used.
     *
     * @param W This is the Width pixel target screen;
     * @param H This is the Height pixel target screen;
     * @param parent This is the parent widget to belong (or null)
     */
    windowsClass(int W, int H, QWidget *parent); //!< Creates a windows based object

    //!Distructor member
    virtual ~windowsClass(){};

    //! Defines the options flags passed to the methods
    typedef enum{
        _WINDOW_NONE        = 0, //!< no options
        _WINDOW_FORCE       = 1, //!< force the window to open even if is already
        _WINDOW_NOINIT      = 2, //!< the Init Callback is not called for the opening windows
        _WINDOW_NOEXIT      = 4, //!< the Exit Callback is not called for the closing windows
    }_WindowFlags;


    /**
    * @brief openWindow This method open the Window
    *
    * This method shall be used in order to open
    * the window of this class instance.
    *
    * When this method is called, the following actions take place:
    * + The windowsClass::exitWindow() of the currently open windows is called;
    * + The the windowsClass::initWindow() of the opening window is called;
    *
    * If the window is already open the method do nothing, unless
    * the option flag windowsClass::_WINDOW_FORCE is passed as parameter:\n
    * in the latter case, the method windowsClass::initWindow() is called anyway.
    *
    * @param opt
    * Bitfield option flags.
    * Available flags are:
    * + windowsClass::_WINDOW_FORCE:  force an already open window to call the windowsClass::initWindow() method;
    * + windowsClass::_WINDOW_NOINIT: prevent to call the windowsClass::initWindow() method;
    * + windowsClass::_WINDOW_NOEXIT: prevent to call the windowsClass::exitWindow() method for the closing window;
    */
    void _inline openWindow(int opt = 0){emit openWindowSgn(opt);}

    /**
    * @brief openParentWindow This method open the parent window
    *
    * The parent window is the last closed window.
    * The class takes the history of the closing windows
    * allowing to retrive the last closed one. This is useful
    * in the case where an asynch window show up for a temporary
    * activity then the previous window shall be ripristinated.
    *
    *
    * @param opt
    * Bitfield option flags.
    * Available flags are:
    * + windowsClass::_WINDOW_NOINIT: prevent to call the windowsClass::initWindow() method for the opening window;
    * + windowsClass::_WINDOW_NOEXIT: prevent to call the windowsClass::exitWindow() method for the closing window;
    */
    void _inline openParentWindow(int opt = 0){emit openParentWindowSgn(opt);}  //!< Open the parent window if there is one

    //! Test if the window is open
    bool isOpen(void) { return (windowsClass::currentWindow == this);}

    void setIncrementRotation(int angolo);   //!< Increment the current View angle
    void setAbsoluteRotation(int angolo);    //!< Set the absolute current View angle

    virtual void initWindow(void); //!< Virtual function for the opening activities
    virtual void exitWindow(void); //!< Virtual function for the exiting activities


signals:
    void openWindowSgn(int opt);    //!< Open window signal to put in the event loop queue a open request
    void openParentWindowSgn(int opt); //!< Parent Open window signal to put in the event loop queue a parent open request

private slots:
    void openWindowSlot(int opt); //!< Open Window slot
    void openParentWindowSlot(int opt);//!< Open Parent Window slot

private:
    static int ID;                  //!< progressive ID of the Class
    windowsClass* parentWindow;           //!< Calling Windows
    static windowsClass* currentWindow;    //!< Current Windows
    static QList<QScreen*> screens; //!< List of availables screens

    int windowID;               //!< Uniq ID associated to the Windows
    QFrame*  workingFrame;      //!< Working frame where are placed all the GUI's Widgets
    QRect screen;               //!< Assigned Screen coordinates and dimension in the virtual desktop

    void setupScene(void);      //!< Init of the SCene and View connecting the Widgets to the scene
    bool viewInit;              //!< Flag used to allow the scene initialization at the first open event
    QGraphicsScene *scene;      //!< Scene used to draw the widget elements
    QGraphicsView *view;        //!< View used over the scene
    QGraphicsProxyWidget *proxy;
    int rotview;                //!< Current rotation view angle

};


#endif // WINDOWS_H
