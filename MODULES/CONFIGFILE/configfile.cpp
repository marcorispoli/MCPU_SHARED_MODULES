#include "configfile.h"

configFile::configFile(const fileDescriptorT descriptor, _cfg_open_mode_t open_mode)
{
    fileDescriptor = descriptor;
    openMode = open_mode;
    fp = new QFile(descriptor.filename);
    if(!fp->exists()) createDefaultFile();

}

/**
 * @brief configFile::loadFile
 *
 * Loads in memory the content of the configuration file.
 *
 * The Load function reads the revision file TAG (if present) and compare it
 * with the fileDescriptor. If the file revision
 * shouldn't match with the expected then the configFile::revisionChangeCallback()
 * virtual function is then called.
 *
 * The load function presets the memory content with the defaults values
 * of the fileDescriptor. In this way every missing parameter is
 * substituted with its default value. In this case
 * the file is then stored to guarantee a correctfile format, based on th fileDescriptor.
 *
 * Either In the case of wrong format or missing parameter, the entire file is stored in memory.
 */
void configFile::loadFile(void){
    paramItemT decodedItem;
    int file_revision = -1;
    bool format_ok = true;

    // Initialize the content of the file with defaults elements
    content = fileDescriptor.descriptor;    // Initialize the content of the file with defaults elements

    // Creates a list of all available tags in order to check if
    // the file has some missing parameter
    QList<QString> tags;
    tags.clear();
    for(int i=0; i< fileDescriptor.descriptor.items.count(); i++){
        tags.append(fileDescriptor.descriptor.items.at(i).tag);
    }

    // Copy the loaded content to a backup copy for further restore
    backup_content = content;

    if(fp == nullptr) return ;
    if(!fp->open(QIODevice::ReadOnly | QIODevice::Text)) return ;


    while(!fp->atEnd())
    {
        // Gets the first availbale param item in the file
        decodedItem = configFile::decodeDataFile(fp);

        // If the item is the file release it is treated separatelly
        if((decodedItem.tag == "REVISION") && (decodedItem.values.data.count() == 1)){
            file_revision = decodedItem.values.data[0].toInt();
            continue;
        }

        // Verifies if the parameter TAG belong to the admitted, checking the fileDescriptor
        int index = getTagPosition(decodedItem.tag);
        if(index < 0){
            format_ok = false;
            continue; // Wrong tag detected
        }

        if(decodedItem.values.data.count() != fileDescriptor.descriptor.items.at(index).values.data.count()){
            format_ok = false;
            continue; // Wrong number of params
        }
        decodedItem.comment = fileDescriptor.descriptor.items.at(index).comment;

        // Checks the param content is correct
        bool param_ok = true;
        for(int k=0; k< decodedItem.values.data.count(); k++){
            if(decodedItem.values.data.at(k).isEmpty()){
                param_ok = false;
                break;
            }
        }
        if(!param_ok) {
            format_ok = false;
            continue;
        }

        content.items[index] = decodedItem;
        tags.removeOne(decodedItem.tag);
    }
    fp->close();

    // Copy the loaded content to a backup copy for further restore
    backup_content = content;

    if((openMode == _CFG_READONLY) && ((file_revision != fileDescriptor.revision) ||(!format_ok))){
        qDebug() << "WRONG CONFIGURATION REVISION OR FORMAT";
        format_ok = false;
        return;
    }
    format_ok = true;

    // Handles the change revision callback
    if(file_revision != fileDescriptor.revision){
        // If the virtual callback should return TRUE then the file is stored
        if(revisionChangeCallback(file_revision, fileDescriptor.revision)) this->storeFile();
    }else if(!format_ok){
        // In case some of the field should not be correctly formatted
        // then the file is restored with the wrong fields modified with
        // the defaults values
        this->storeFile();
    }

    return ;
}

/**
 * @brief configFile::storeFile
 * This function stores the memory content of the configuration file.
 *
 * The file is renewed preserving the fileDescriptor structure.
 * The revision tag is wrote at the top of the file.
 * At the end, the backup content is updated with the current file content.
 *
 *
 */
void configFile::storeFile(void){
    if(!fp->open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    // Create the revision tag in the top of the file
    paramItemT item;
    item.tag = "REVISION";
    item.comment = "revision file";
    item.values.data.clear();
    item.values.data.append(QString("%1").arg(fileDescriptor.revision));
    fp->write(configFile::encodeDataFile(&item).toLatin1().data());

    // Store the content to tyhe file
    for(int i=0; i< content.items.count(); i++){
        fp->write(configFile::encodeDataFile(&(content.items[i])).toLatin1().data());
    }
    fp->flush();
    fp->close();

    backup_content = content;
    return;
}

/**
 * @brief configFile::createDefaultFile
 *
 * This function replace the file (or create a new one)
 * copying the content of the descriptorFile
 * to the file system.
 */
void configFile::createDefaultFile(void){
    if(!fp->open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    paramItemT item;

    // Create the revision tag
    item.tag = "REVISION";
    item.comment = "revision file";
    item.values.data.clear();
    item.values.data.append(QString("%1").arg(fileDescriptor.revision));
    fp->write(configFile::encodeDataFile(&item).toLatin1().data());

    for(int i=0; i< fileDescriptor.descriptor.items.count(); i++){
        fp->write(configFile::encodeDataFile(&(fileDescriptor.descriptor.items[i])).toLatin1().data());
    }
    fp->flush();
    fp->close();

    content = fileDescriptor.descriptor;
    backup_content = content;
}

int configFile::getTagPosition(QString tag){
    for(int i=0; i<fileDescriptor.descriptor.items.count(); i++){
        if(fileDescriptor.descriptor.items.at(i).tag == tag) return i;
    }
    return -1;
}

QString configFile::encodeDataFile(paramItemT* item){
    if(item == nullptr) return "";
    QString stringa="";

    // Insert a comment line before data content
    if(!item->comment.isEmpty()) stringa +="# " + QString(item->comment) + "\n";

    stringa += "<" + QString(item->tag) ;
    for(int i=0; i< item->values.data.count(); i++) stringa += "," + item->values.data.at(i) ;
    stringa += ">\n";
    return stringa;
}

configFile::paramItemT configFile::decodeDataFile(QFile* fp)
{
    paramItemT item;
    bool isTag = true;
    int i;

    QByteArray frame = configFile::getNextValidLine(fp) ;
    frame.replace(" ","");

    item.values.data.clear();
    item.comment = "";
    item.tag = "";
    while(frame.size())
    {
        i = frame.indexOf(",");
        if(i==-1)
        {
            if(frame.isEmpty())  return item;
            item.values.data.append(frame);
            return item;
        }

        if(isTag) item.tag = frame.left(i);
        else item.values.data.append(frame.left(i));
        isTag = false;

        if(i==frame.size()) break;
        frame = frame.right(frame.size()-i-1);
    }
    return item;
}

QByteArray configFile::getNextValidLine(QFile* fp)
{
    QByteArray risultato, frame;
    int i;

    // Scansione righe del file
    while(!fp->atEnd())
    {
        frame = fp->readLine(); // Lettura riga
        risultato.clear();

        // Se primo caratter ==  # salta la linea
        if(frame.at(0)=='#') continue;

        // Ricerca carattere '<'
        for(i=0; i<frame.size(); i++)  if(frame.at(i)=='<') break;
        if(i==frame.size()) continue; // Nessun carattere trovato, passa a nuova riga
        i++; // i contiene il primo carattere valido

        // Costruzione dato fino a  '>' o a ','
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>') break;
            else risultato.append(frame.at(i));
        }
        if(i==frame.size()) continue; // Nessun carattere trovato, passa a nuova riga
        if(risultato.size()==0) continue;
        // Risultato contiene la riga valida
        return risultato;
    }

    // Nessuna nuova riga trovata
    risultato.clear();
    return risultato;
}
