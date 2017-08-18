#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl httpUrl, QObject *parent) :
    QObject(parent)
{
    QNetworkRequest request(httpUrl);
    reply = m_WebCtrl.get(request);

    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(emitDownloadProgress(qint64,qint64)));
    connect(&m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
            this, SLOT (fileDownloaded(QNetworkReply*)) );

}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
    m_DownloadedData = pReply->readAll();
    //emit a signal
    pReply->deleteLater();
    emit downloaded();
}

void FileDownloader::httpReadyRead()
{

}

QByteArray FileDownloader::downloadedData() const {
    return m_DownloadedData;
}

void FileDownloader::emitDownloadProgress(qint64 recvSize, qint64 totalSize)
{
    emit downloadProgress(recvSize, totalSize);
}
