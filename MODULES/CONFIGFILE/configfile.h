#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <QObject>
#include <QFile>
#include <QVariant>
/**
 * \defgroup  configModule Configuration file Module Library
 * \ingroup libraryModules
 *
 * # ABSTRACT
 *
 * The configuration file is a special file with a proper
 * format: a string based file with TAGS and a variable set of parameters.
 *
 * The basic characteristics are:
 *
 *  + A Readable Text file format;
 *  + A robust data structure and access data interface, preventing data corruption;
 *  + Comment strings inside the file allow the readability;
 *  + Etherogeneous data content: all data are in string format convertible to any kind of data types.
 *  + File revision management.
 *
 * # FILE DATA FORMAT
 *
 * The file is composed of several string lines, each one
 * handling a given parameter with a list of values.
 *
 * Each parameter is defined by a static string as a TAG identifier.
 * The following string show the data format:
 * \verbatim

  # Comment line ......
  <TAG,PARAM1, PARAM2, PARAM3,...., PARAM-N>

  where:
  - # is a comment initiator: every character following the '#' is interpreted as a comment line.
  - TAG: is a unique string identifier. This string shall be used in the code in order to point to a parameter.
  - PARAM1, .., PARAM-N: are strings separated by commas, representing the parameter content,
    splitted in a list of etherogeneus values in string format.
  \endverbatim
 *
 * A special TAG = "REVISION" is reserved to store the file revision code in the file.
 * This tag is used to determines if the configuration file is at the correct
 * revision when the application accesses the file.
 * A virtual configFile::revisionChangeCallback() function is called in case a wrong revision
 * should be detected.
 *
 * # USAGE
 *
 * The application shall subclass this class in order to create a
 * specific configuration file.
 *
 * In the constructor of the subclass the application shall pass a static structure
 * defining the configFile::fileDescriptorT of the configuration file.
 * The fileDescriptor content defines:
 * - The list of available parameter TAGS;
 * - The default content of every parameter;
 * - The description string assigned to every parameter;
 *
 * This is a tipical valid declaration:
 \verbatim
    class yourConfigClass: public configFile
    {
        public:


        #define REVISION     1  // This is the revision code
        #define CONFIG_FILENAME     "C:\\Users\\.....\\your_config_file" // This is the configuration file name and path

        // This section defines labels helping the param identification along the application
        #define NAME_TAG_1   "NAME_TAG_1"
        #define NAME_TAG_2   "NAME_TAG_2"
        ....
        #define NAME_TAG_K   "NAME_TAG_K"

        // your class constructor
        yourConfigClass():configFile( (const configFile::fileDescriptorT)
            {
                CONFIG_FILENAME, REVISION,
                {{
                    { NAME_TAG_1,     {{ VAL_1_1, ... , VAL_1_N }},  "Description string tag 1"},
                    { NAME_TAG_2,     {{ VAL_2_1, ... , VAL_2_O }},  "Description string tag 2"},
                    { NAME_TAG_K,     {{ VAL_K_1, ... , VAL_K_P }},  "Description string tag K"},
                }}
            })
        {
            // Your constructor code ...
            this->loadFile();
        };


    }; // End class definition
\endverbatim
 *  Override the configFile::revisionChangeCallback() function to customize the\n
    default change revision callback
    \verbatim
     bool yourConfigClass::changeRevisionCallback(int file_rev, int config_rev){

     // your code here .....

     return true; // if you want to store the file at the end
     return false; // to prevent the file storing
    }
 \endverbatim
 *
 * As the previous code example shows, the class declaration
 * needs a "descriptor" structure, configFile::fileDescriptorT, that uniquelly defines the
 * content of the configuration file.
 *
 * When the configuration file will be instantiated for the first time,
 * a configuration file called "your_config_file" will be created with
 * the default values taken from the configFile::fileDescriptor.
 *
 * Further instantiation of the class will not modifies the content of the file.
 *
 * After instantiation, the memory content will be filled with the default values.\n
 * In order to update the memory content with the current file content, \n
 * the application shall explicitely call the configFile::loadFile() method.
 *
 * In case the file content should have some wrong formatted parameter line,
 * or some of the tag should not be present,\n
 * the wrong formatted lines will be removed from the file and the missing parameters\n
 * will be replaced with the default values.
 * The file then will be rigenerated with the corrected format.
 *
 * To access the configuration data content, the application shall use the following
 * methods:
 * \verbatim
 *
   // class instantiation in some part of the code
   yourConfigClass config;

   // Optionally the application shall load the content in memory
   // (if this as not been done in the yourConfigClass constructor yet)
   config.loadFile();

   // Access to a whole value list of a parameter
   paramItemT param = getParam("NAME_TAG_1");
     // param.tag: is the tag of the parameter;
     // param.values.at(i): is the i-value of the value list in string format

   // Access to the first element of the value's list of a parameter
   int val = getParam<int>("NAME_TAG_1", 0);


   // Setting the n-value value of a given param:
   setParam<int>("NAME_TAG_1",index); // For integers
   setParam<float>("NAME_TAG_1",index); // For floats
   .....
   setParam<QString>("NAME_TAG_1",index); // For strings
 \endverbatim

 * In all of the previous access methods, the application shall\n
 * check the valid data access in order to proceed. See configFile::isAccess()
 *
 * The class takes a copy of the loaded file.
 * After content modifications with the setParam() method,
 * the application can restore the original content
 * using the public method configFile::restoreContent().
 * This methode doesn't change the file in the file system, but
 * restore te content in memory.
 * In order to restore the file the application should call
 * both methods:
 * + configFile::restoreContent();
 * + configFile::storeFile();
 *
 * The application can load the file, store the file, reset to default
 * value the file with the following public methods:
 * - configFile::loadFile();
 * - configFile::storeFile();
 * - configFile::setDefaultFile();
 *
 *
 *
 */

/**
 * \ingroup configModule
 *
 * @brief Configuration PArent class
 *
 * The application specific configuration file shall derive from this class
 *
 */
class configFile
{
public:

    #define _CFGDEF(x) {{x}}

    typedef enum{
        _CFG_READONLY = 0,
        _CFG_READWRITE
    }_cfg_open_mode_t;

    //!> Parameter value definition structure
    typedef struct{
        QList<QString> data;
    }paramValueT;

    //! Prameter Item content structure
    typedef struct {
        QString tag;            //!< Tag name of the parameter
        paramValueT values;  //!< Value list of a parameter
        QString comment;        //!< comment
    }paramItemT;

    //! Structure of the configuratioon file data content
    typedef struct{
        QList<paramItemT> items;
    } paramItemContentT;

    //! Configuration file descriptor
    typedef struct {
        const char* filename;
        int revision;
        paramItemContentT descriptor;
    }fileDescriptorT;



    /**
     * @brief configFile constructo class
     *
     * Instantiating this class automatically generates a file
     * with the parameter structure defined into the descriptor field.
     *
     * @param descriptor
     * This is the descriptor structure of the configuration file
     *
     */
    configFile( const fileDescriptorT descriptor, _cfg_open_mode_t open_mode = _CFG_READWRITE);

    //! Load the configuration file in memory
    void loadFile(void);

    //! Stores the memory content of the configuration file into the file.
    void storeFile(void);

    //! Restores the memory content to the one just after the file loading
    _inline void restoreContent(void){ content = backup_content; }

    //! Override thecurrent file with the default values
    _inline void setDefaultFile(void){ createDefaultFile(); }

    /**
     * @brief getParam
     * Gets the value list of a given parameter from the memory content.
     * @param tag
     * This is the tag name of the parameter
     * @return
     * paramValueT value list of the tagged parameter.
     * @attention
     * The application shall check the configFile::isAccess() to check
     * the correctness of the data access.
     */
    paramValueT getParam(const char* tag){
        int i = getTagPosition(tag);
        if(i < 0){
            dataAccess = false;
            return paramValueT {};
        }

        dataAccess = true;
        return content.items.at(i).values;
    }

    /**
     * @brief getParam
     *
     * This function returns the i-value item of the target parameter.
     * @param tag
     * Tag name of the parameter;
     * @param index
     * index of the parameter in the list
     *
     * @return
     * T type custed data content of the i element of value list of a parameter;
     * @attention
     * The application shall check the configFile::isAccess() to check
     * the correctness of the data access.
     */
    template <typename T> T getParam(const char* tag, int index){

        int i = getTagPosition(tag);
        if((i < 0) || (index < 0) || (index >= content.items.at(i).values.data.count()))
        {
            dataAccess = false;
            return QVariant("0").value<T>();
        }
        dataAccess = true;
        return QVariant(content.items[i].values.data.at(index)).value<T>();
    }

    //! Check if the last data access is valid
    _inline bool isAccess(void){return dataAccess;}

    /**
     * @brief setParam
     * This function set the content of an item of the value list of a given parameter.
     *
     * @param tag
     * This is the parameter name
     * @param val
     * This is the value of an arbitrary type.
     * @param index
     * this is the index of the value list of the parameter.
     * @attention
     * The function shall be called declaring the data Type
     * in the function template: setPar<Type>(..)
     *
     * The application shall check the configFile::isAccess() to check
     * the correctness of the data access.
     */
    template <typename T> void setParam(const char* tag, int index, T val){
        int i = getTagPosition(tag);
        if( (i < 0) || (index >= content.items.at(i).values.data.count())) {
            dataAccess = false;
            return ;
        }

        dataAccess = true;
        content.items[i].values.data[index] = QString("%1").arg(val);
        return ;
    }

    /**
     * @brief revisionChangeCallback
     * This callback is called whenever a file config results of different revision than expected.
     *
     * Reimplement this function in order to handle the differences
     * @param filerev
     * The file revision
     * @param rev
     * The expected revision
     * @return
     * Return true from this callback to allow the class to store the
     * file with the updated content.
     */
    virtual bool revisionChangeCallback(int filerev, int rev){

        return true;
    }

    bool inline isFormatCorrect(void){return format_ok;}

private:
    QFile* fp; //!< file handle
    _cfg_open_mode_t openMode; //!< Read and write opening mode
    bool format_ok; //!< The configuration file has been correctly uploaded

    paramItemContentT content; //!< config file content in memory
    paramItemContentT backup_content;//!< backup of the content before any modification
    fileDescriptorT fileDescriptor; //!< descriptor of the internal file params structure
    bool dataAccess; //!< result of the last data access

    /**
     * @brief getTagPosition
     * Get the index position of a given tag in the memory structure
     * @param tag
     * tag name
     * @return
     */
    int getTagPosition(QString tag);
    void createDefaultFile(void); //!< Creates the default file based on the template

    static QByteArray   getNextValidLine(QFile* fp); //!< Reads a line in the file and returns the first formatted line
    static QString      encodeDataFile(paramItemT* item); //!< Encode an item in a string to be written into the file
    static paramItemT   decodeDataFile(QFile* fp); //!< Decode a parameter string read from the file
};

#endif // CONFIGFILE_H
