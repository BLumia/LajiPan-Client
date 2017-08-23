#include "filedownloader.h"
#include <QApplication>

FileDownloader::FileDownloader(QUrl httpUrl, QObject *parent) :
    QObject(parent)
{
    fileName = "debug.dl";
    this->httpUrl = httpUrl;

    connect(&m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
            this, SLOT (fileDownloaded(QNetworkReply*)) , Qt::UniqueConnection);
}

FileDownloader::~FileDownloader() {
    //m_DownloadedData.resize(0); // try to fix malloc problem?
}

void FileDownloader::startDownload()
{
    qDebug() << "Start downloading:" << httpUrl;
    QNetworkRequest request(httpUrl);
    reply = m_WebCtrl.get(request);

    eventLoop = new QEventLoop;

    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()), Qt::UniqueConnection);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(emitDownloadProgress(qint64,qint64)), Qt::UniqueConnection);

    //eventLoop->exec();
}

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
    m_DownloadedData = pReply->readAll();
    //emit a signal
    pReply->deleteLater();
    //delete pReply; // fuck you!
    QApplication::processEvents();
    this->eventLoop->quit();
    emit downloaded();
}

void FileDownloader::httpReadyRead()
{
    if (fileName.compare("debug.dl") != 0) return;

    QByteArray fileDisposition = this->reply->rawHeader(QByteArray("Content-Disposition"));
    int quoteStart = -1, quoteEnd = -1;
    for (int i = 0; i != fileDisposition.length (); i++) {
        if (fileDisposition[i] == '"') {
            if (quoteStart == -1) {
                quoteStart = i + 1;
            } else {
                quoteEnd = i;
                break;
            }
        }
    }
    fileName = QString(fileDisposition).mid(quoteStart, quoteEnd - quoteStart);
    qDebug() << "httpReadyRead: " << fileName;
}

QByteArray FileDownloader::downloadedData() /*const*/ {
//    QByteArray ret(m_DownloadedData);
//    m_DownloadedData.clear();
//    return ret;
    return m_DownloadedData;
}

void FileDownloader::emitDownloadProgress(qint64 recvSize, qint64 totalSize)
{
    emit downloadProgress(recvSize, totalSize);
}
