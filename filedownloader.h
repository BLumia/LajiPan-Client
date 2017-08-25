#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QUrl>

class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QUrl httpUrl, QObject *parent = 0) : QObject(parent) {
        Q_UNUSED(httpUrl);
    }
    ~FileDownloader() {}

    virtual void startDownload() = 0;
    virtual QByteArray downloadedData() = 0;
    virtual QString getFileName() = 0;

signals:
    void downloaded();
    void downloadProgress(qint64, qint64);

};

#endif // FILEDOWNLOADER_H
