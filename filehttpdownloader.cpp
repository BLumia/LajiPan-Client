#include "filehttpdownloader.h"

FileHTTPDownloader::FileHTTPDownloader(QUrl httpUrl, QObject *parent) :
    FileDownloader(httpUrl, parent)
{
    fileName = "debug.dl";
    this->httpUrl = httpUrl;
    m_WebCtrl = new QNetworkAccessManager();

    connect(m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
            this, SLOT (fileDownloaded(QNetworkReply*)) , Qt::UniqueConnection);
}

void FileHTTPDownloader::startDownload()
{
    qDebug() << "Start downloading:" << httpUrl;
    QNetworkRequest request(httpUrl);
    reply = m_WebCtrl->get(request);

    //eventLoop = new QEventLoop;

    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()), Qt::UniqueConnection);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(emitDownloadProgress(qint64,qint64)), Qt::UniqueConnection);

    //eventLoop->exec();
}

QByteArray FileHTTPDownloader::downloadedData()
{
    return m_DownloadedData;
}

QString FileHTTPDownloader::getFileName()
{
    return fileName;
}

void FileHTTPDownloader::emitDownloadProgress(qint64 recvSize, qint64 totalSize)
{
    emit downloadProgress(recvSize, totalSize);
}

void FileHTTPDownloader::fileDownloaded(QNetworkReply *pReply)
{
    m_DownloadedData = pReply->readAll();
    //emit a signal
    pReply->deleteLater();
    //delete pReply; // fuck you!
    m_WebCtrl->disconnect();
    delete m_WebCtrl;
    //QApplication::processEvents();
    //this->eventLoop->quit();
    emit downloaded();
}

void FileHTTPDownloader::httpReadyRead()
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
