#ifndef BLOCKINGDOWNLOADER_H
#define BLOCKINGDOWNLOADER_H

#include "lajiutils.h"

#include <QObject>
#include <QThread>

class BlockingDownloader : public QThread
{
    Q_OBJECT
public:
    BlockingDownloader(QAddressPort addrPort, int32_t chunkID);
    QByteArray m_DownloadedData;

private:
    QAddressPort addrPort;
    int32_t chunkID;

    void run();

signals:
    void gotFileName(QString);
    void downloadProgress(qint64, qint64);
    void downloaded();
};

#endif // BLOCKINGDOWNLOADER_H
