#ifndef FILETCPBLOCKINGDOWNLOADER_H
#define FILETCPBLOCKINGDOWNLOADER_H

#include "blockingdownloader.h"
#include "filedownloader.h"
#include "lajiutils.h"

#include <QByteArray>


class FileTcpBlockingDownloader : public FileDownloader
{
    Q_OBJECT
public:
    explicit FileTcpBlockingDownloader(QUrl httpUrl, QObject *parent = 0);
    ~FileTcpBlockingDownloader();
    void startDownload();
    QByteArray downloadedData() /*const*/;
    QString getFileName();

signals:
    void downloaded();
    void downloadProgress(qint64, qint64);

private:
    BlockingDownloader* blockingDownloader;
    QAddressPort addrPort;
    QDataStream in;
    QByteArray m_DownloadedData;
    qint64 totalBytes = -1;
    int32_t chunkID;
    QString fileName;

private slots:
    void setFileName(QString fileName);
    void onDownloadDone();
};

#endif // FILETCPBLOCKINGDOWNLOADER_H
