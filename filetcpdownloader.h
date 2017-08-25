#ifndef FILETCPDOWNLOADER_H
#define FILETCPDOWNLOADER_H

#include "filedownloader.h"
#include "lajiutils.h"

#include <QObject>
#include <QByteArray>
#include <QTcpSocket>
#include <QUrl>

class FileTCPDownloader : public FileDownloader
{
    Q_OBJECT
public:
    explicit FileTCPDownloader(QUrl httpUrl, QObject *parent = 0);
    ~FileTCPDownloader();
    void startDownload();
    QByteArray downloadedData() /*const*/;
    QString getFileName();

signals:
    void downloaded();
    void downloadProgress(qint64, qint64);

public slots:

private:
    QAddressPort addrPort;
    QDataStream in;
    QTcpSocket *tcpSocket;
    QByteArray m_DownloadedData;
    qint64 totalBytes = -1;
    int32_t chunkID;
    QString fileName;

private slots:
    void socketReadyRead();
    void sendDownloadRequest();
};

#endif // FILETCPDOWNLOADER_H
