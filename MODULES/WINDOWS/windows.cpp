#include "windows.h"


// Static instance of the static variable of the class
int windowsClass::ID = 0;
windowsClass* windowsClass::currentWindow = nullptr;
QList<QScreen*> windowsClass::screens;


windowsClass::windowsClass(int W, int H, QWidget *parent) : QWidget(parent)
{

    // Get the corresponding screen, based on the resolution data
    // If no resolution is matched then it is used the first screen in the order
    if(windowsClass::ID == 0){
        windowsClass::screens = QApplication::screens();
    }

    screen = windowsClass::screens[0]->geometry();

    for(int i=0; i<windowsClass::screens.size(); i++){
        if( (windowsClass::screens[i]->geometry().height() == H) && (windowsClass::screens[i]->geometry().width() == W)){
             screen = windowsClass::screens[i]->geometry();
             break;
        }
    }

    // This flag is used to initialize only once the View at the first openWindow()
    viewInit = false;
    rotview = 0;

    // Assign the proper ID
    windowID = ID;
    windowsClass::ID++;

    this->parentWindow = nullptr; // No parent window when created

    // Connect the open commands to Queued slots
    connect(this,SIGNAL(openWindowSgn(int)), this, SLOT(openWindowSlot(int)), Qt::QueuedConnection);
    connect(this,SIGNAL(openParentWindowSgn(int)), this, SLOT(openParentWindowSlot(int)), Qt::QueuedConnection);

}


/**
 * This Slot is activated by the Event Queue executing
 * a queued windowsClass::openWindow() signal
 *
 * The following actions take place:
 * + The windowsClass::exitWindow() of the closing window is called;
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
void windowsClass::openWindowSlot(int opt){

    // Se non è stata ancora inizializzata la grafica lo fa ora
    if(!viewInit) setupScene();

    // Condition for already open windows and force option
    if(this == windowsClass::currentWindow){
        // Force the init again
        if(opt & _WINDOW_FORCE) initWindow();
        return;
    }

    // Before to open the new windows, close the current one
    if(windowsClass::currentWindow != nullptr){

        // Exit activities for the current window
        if(!(opt & _WINDOW_NOEXIT)) windowsClass::currentWindow->exitWindow();

        // Hide the Window
        windowsClass::currentWindow->view->hide();
    }

    // Assignes the parent pointer to the current opening windows
    this->parentWindow = windowsClass::currentWindow;
    windowsClass::currentWindow = this;

    // Run the overridable function to initialization
    if(!(opt & _WINDOW_NOINIT)) initWindow();

    // Shows the current window
    view->show();
}

/**
 * This function opens the parent window if there is one
 *
 * If no parent window is present, the function do nothing
 * @param opt
 */
void windowsClass::openParentWindowSlot(int opt){

    // No parent present
    if(this->parentWindow == nullptr) return;

    // Post open the parent windows
    this->parentWindow->openWindow(opt);
}

/**
 * This function creates the Scene and View
 * including the Widgets in it. This is necessary
 * in order to handle graphics actions like rotation.
 *
 * This method is called only at the first window opening
 * and not into the creator method, because the Widgets are created
 * in order after this class. So it is necessary to call this section
 * only after the Subclass is completelly created.
 *
 */
void windowsClass::setupScene(void){
    viewInit = true;

    // Find the working frame of the Widget
    workingFrame = this->findChild<QFrame *>();
    if(workingFrame == NULL) return;

    // Scene creation: the gui widget are connected to te scene
    scene = new QGraphicsScene();
    int w = windowsClass::screen.width();
    int h = windowsClass::screen.height();
    int x = windowsClass::screen.x();
    int y = windowsClass::screen.y();
    scene->setSceneRect(x,y,w,h);
    proxy = scene->addWidget(this);

    // Set the geometry of the Widget cenetered to the screen and 2 times wider than the workingFrame
    w = workingFrame->width()*2;
    h = workingFrame->height()*2;
    x = windowsClass::screen.x() + 0.5 * (windowsClass::screen.width() - w);
    y = windowsClass::screen.y() + 0.5 * (windowsClass::screen.height() - h);
    this->setGeometry(x,y,w,h);

    // Set the working frame in the center of the Widget area
    w = workingFrame->width();
    h = workingFrame->height();
    x = 0.5 * (this->width() - w);
    y = 0.5 * (this->height() - h);
    workingFrame->setGeometry(x,y,w,h);

    // Set the view
    view = new QGraphicsView(scene);
    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setAlignment(Qt::AlignCenter);
    view->setScene(scene);

    // Center the View on the Widget area
    w = workingFrame->width();
    h = workingFrame->height();
    x = this->x() + 0.5 * (this->width() - w);
    y = this->y() + 0.5 * (this->height() - h);
    view->setGeometry(x,y,w,h);

}

/**
 * The user should implement this virtual
 * method in order to execute initialization
 * code before to open the Window.
 *
 * This function is not called in the case the
 * application should use windowsClass::_WINDOW_NOINIT
 * in the opening method.
 *
 * The code executes before the GUI shows up
 */
void windowsClass::initWindow(void){

    // The base method do nothing!
    return;
}

/**
 * The user should implement this virtual
 * method in order to execute exit activities
 * before to exit the Window.
 *
 * This function is not called in the case the
 * application should use windowsClass::_WINDOW_NOEXIT
 * in the opening method.
 *
 * The code executes before the GUI hides
 */
void windowsClass::exitWindow(void){

    // The base method do nothing!
    return;
}

/**
 * This method allows to rotate the current view of a given angle
 * from the previos angle view.
 *
 * @param angolo relative increment of the current View angle
 */
void windowsClass::setIncrementRotation(int angolo){

    rotview += angolo;
    view->rotate(angolo);
    view->centerOn(windowsClass::screen.x() + windowsClass::screen.width() *0.5, windowsClass::screen.y() + windowsClass::screen.height() *0.5);
}

/**
 * This method allows to rotate the current view of a given angle
 * from the previos angle view.
 *
 * @param angolo absolute View angle in degree
 */
void windowsClass::setAbsoluteRotation(int angolo){


    view->rotate(angolo - rotview);
    rotview = angolo;
    view->centerOn(windowsClass::screen.x() + windowsClass::screen.width() *0.5, windowsClass::screen.y() + windowsClass::screen.height() *0.5);
}

