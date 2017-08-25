#ifndef FILEHTTPDOWNLOADER_H
#define FILEHTTPDOWNLOADER_H

#include "filedownloader.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

class FileHTTPDownloader : public FileDownloader
{
    Q_OBJECT
public:
    explicit FileHTTPDownloader(QUrl httpUrl, QObject *parent = 0);
    void startDownload();
    QByteArray downloadedData() /*const*/;
    QString getFileName();

signals:

private slots:
    void emitDownloadProgress(qint64 recvSize, qint64 totalSize);
    void fileDownloaded(QNetworkReply* pReply);
    void httpReadyRead();

private:
    QNetworkAccessManager* m_WebCtrl;
    QNetworkReply *reply;
    QByteArray m_DownloadedData;
    QUrl httpUrl;
    //QEventLoop* eventLoop;
    QString fileName;
};

#endif // FILEHTTPDOWNLOADER_H
