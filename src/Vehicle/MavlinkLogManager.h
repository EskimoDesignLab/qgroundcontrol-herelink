/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#ifndef MavlinkLogManager_H
#define MavlinkLogManager_H

#include <QObject>

#include "QmlObjectListModel.h"
#include "QGCLoggingCategory.h"
#include "QGCToolbox.h"
#include "Vehicle.h"

Q_DECLARE_LOGGING_CATEGORY(MavlinkLogManagerLog)

class QNetworkAccessManager;
class MavlinkLogManager;

//-----------------------------------------------------------------------------
class MavlinkLogFiles : public QObject
{
    Q_OBJECT
public:
    MavlinkLogFiles    (MavlinkLogManager* manager, const QString& filePath, bool newFile = false);

    Q_PROPERTY(QString      name        READ    name                                CONSTANT)
    Q_PROPERTY(quint32      size        READ    size                                NOTIFY sizeChanged)
    Q_PROPERTY(bool         selected    READ    selected    WRITE setSelected       NOTIFY selectedChanged)
    Q_PROPERTY(bool         uploading   READ    uploading                           NOTIFY uploadingChanged)
    Q_PROPERTY(qreal        progress    READ    progress                            NOTIFY progressChanged)
    Q_PROPERTY(bool         writing     READ    writing                             NOTIFY writingChanged)
    Q_PROPERTY(bool         uploaded    READ    uploaded                            NOTIFY uploadedChanged)

    QString     name                () { return _name; }
    quint32     size                () { return _size; }
    bool        selected            () { return _selected; }
    bool        uploading           () { return _uploading; }
    qreal       progress            () { return _progress; }
    bool        writing             () { return _writing; }
    bool        uploaded            () { return _uploaded; }

    void        setSelected         (bool selected);
    void        setUploading        (bool uploading);
    void        setProgress         (qreal progress);
    void        setWriting          (bool writing);
    void        setSize             (quint32 size);
    void        setUploaded         (bool uploaded);

signals:
    void        sizeChanged         ();
    void        selectedChanged     ();
    void        uploadingChanged    ();
    void        progressChanged     ();
    void        writingChanged      ();
    void        uploadedChanged     ();

private:
    MavlinkLogManager*  _manager;
    QString             _name;
    quint32             _size;
    bool                _selected;
    bool                _uploading;
    qreal               _progress;
    bool                _writing;
    bool                _uploaded;
};

//-----------------------------------------------------------------------------
class MavlinkLogProcessor
{
public:
    MavlinkLogProcessor();
    ~MavlinkLogProcessor();
    void                close       ();
    bool                valid       ();
    bool                create      (MavlinkLogManager *manager, const QString path, uint8_t id);
    MavlinkLogFiles*    record      () { return _record; }
    QString             fileName    () { return _fileName; }
    bool                processStreamData(uint16_t _sequence, uint8_t first_message, QByteArray data);
private:
    bool                _checkSequence(uint16_t seq, int &num_drops);
    QByteArray          _writeUlogMessage(QByteArray &data);
    void                _writeData(void* data, int len);
private:
    FILE*               _fd;
    quint32             _written;
    int                 _sequence;
    int                 _numDrops;
    bool                _gotHeader;
    bool                _error;
    QByteArray          _ulogMessage;
    QString             _fileName;
    MavlinkLogFiles*    _record;
};

//-----------------------------------------------------------------------------
class MavlinkLogManager : public QGCTool
{
    Q_OBJECT

public:
    MavlinkLogManager    (QGCApplication* app);
    ~MavlinkLogManager   ();

    Q_PROPERTY(QString              emailAddress        READ    emailAddress        WRITE setEmailAddress       NOTIFY emailAddressChanged)
    Q_PROPERTY(QString              description         READ    description         WRITE setDescription        NOTIFY descriptionChanged)
    Q_PROPERTY(QString              uploadURL           READ    uploadURL           WRITE setUploadURL          NOTIFY uploadURLChanged)
    Q_PROPERTY(bool                 enableAutoUpload    READ    enableAutoUpload    WRITE setEnableAutoUpload   NOTIFY enableAutoUploadChanged)
    Q_PROPERTY(bool                 enableAutoStart     READ    enableAutoStart     WRITE setEnableAutoStart    NOTIFY enableAutoStartChanged)
    Q_PROPERTY(bool                 deleteAfterUpload   READ    deleteAfterUpload   WRITE setDeleteAfterUpload  NOTIFY deleteAfterUploadChanged)
    Q_PROPERTY(bool                 uploading           READ    uploading                                       NOTIFY uploadingChanged)
    Q_PROPERTY(bool                 logRunning          READ    logRunning                                      NOTIFY logRunningChanged)
    Q_PROPERTY(bool                 canStartLog         READ    canStartLog                                     NOTIFY canStartLogChanged)
    Q_PROPERTY(QmlObjectListModel*  logFiles            READ    logFiles                                        NOTIFY logFilesChanged)

    Q_INVOKABLE void uploadLog      ();
    Q_INVOKABLE void deleteLog      ();
    Q_INVOKABLE void cancelUpload   ();
    Q_INVOKABLE void startLogging   ();
    Q_INVOKABLE void stopLogging    ();

    QString     emailAddress        () { return _emailAddress; }
    QString     description         () { return _description; }
    QString     uploadURL           () { return _uploadURL; }
    bool        enableAutoUpload    () { return _enableAutoUpload; }
    bool        enableAutoStart     () { return _enableAutoStart; }
    bool        uploading                ();
    bool        logRunning          () { return _logRunning; }
    bool        canStartLog         () { return _vehicle != NULL; }
    bool        deleteAfterUpload   () { return _deleteAfterUpload; }

    QmlObjectListModel* logFiles    () { return &_logFiles; }

    void        setEmailAddress     (QString email);
    void        setDescription      (QString description);
    void        setUploadURL        (QString url);
    void        setEnableAutoUpload (bool enable);
    void        setEnableAutoStart  (bool enable);
    void        setDeleteAfterUpload(bool enable);

    // Override from QGCTool
    void        setToolbox          (QGCToolbox *toolbox);

signals:
    void emailAddressChanged        ();
    void descriptionChanged         ();
    void uploadURLChanged           ();
    void enableAutoUploadChanged    ();
    void enableAutoStartChanged     ();
    void logFilesChanged            ();
    void selectedCountChanged       ();
    void uploadingChanged           ();
    void readyRead                  (QByteArray data);
    void failed                     ();
    void succeed                    ();
    void abortUpload                ();
    void logRunningChanged          ();
    void canStartLogChanged         ();
    void deleteAfterUploadChanged   ();

private slots:
    void _uploadFinished            ();
    void _dataAvailable             ();
    void _uploadProgress            (qint64 bytesSent, qint64 bytesTotal);
    void _activeVehicleChanged      (Vehicle* vehicle);
    void _mavlinkLogData            (Vehicle* vehicle, uint8_t target_system, uint8_t target_component, uint16_t sequence, uint8_t first_message, QByteArray data, bool acked);
    void _armedChanged              (bool armed);
    void _commandLongAck            (uint8_t compID, uint16_t command, uint8_t result);
    void _processCmdAck             ();

private:
    bool _sendLog                   (const QString& logFile);
    bool _processUploadResponse     (int http_code, QByteArray &data);
    bool _createNewLog              ();
    int  _getFirstSelected          ();
    void _insertNewLog              (MavlinkLogFiles* newLog);
    void _deleteLog                 (MavlinkLogFiles* log);
    void _discardLog                ();
    QString _makeFilename           (const QString& baseName);

private:
    QString                 _description;
    QString                 _emailAddress;
    QString                 _uploadURL;
    QString                 _logPath;
    bool                    _enableAutoUpload;
    bool                    _enableAutoStart;
    QNetworkAccessManager*  _nam;
    QmlObjectListModel      _logFiles;
    MavlinkLogFiles*        _currentLogfile;
    Vehicle*                _vehicle;
    bool                    _logRunning;
    bool                    _loggingDisabled;
    MavlinkLogProcessor*    _logProcessor;
    bool                    _deleteAfterUpload;
    int                     _loggingCmdTryCount;
    QTimer                  _ackTimer;
};

#endif