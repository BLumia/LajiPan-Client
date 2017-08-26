#ifndef FILEUPLOADER_H
#define FILEUPLOADER_H

#include "icucmodel.h"

#include <QObject>
#include <QTcpSocket>
#include <QThread>

class FileUploader : public QThread
{
    Q_OBJECT
public:
    explicit FileUploader(QAddressPort addrPort, QString filePath, QString &md5Str, ICucModel &icucData);

private:
    QAddressPort addrPort;
    QString filePath;
    QString md5Str;
    ICucModel icucData;
    void run();

signals:
    void uploaded();
    void uploadProgress(qint64, qint64);
};

#endif // FILEUPLOADER_H
