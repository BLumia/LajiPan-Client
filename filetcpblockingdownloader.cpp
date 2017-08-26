#include "filetcpblockingdownloader.h"

FileTcpBlockingDownloader::FileTcpBlockingDownloader(QUrl httpUrl, QObject *parent) :
    FileDownloader(httpUrl, parent)
{
    // for some reason we still choose to write a adapter for http
    fileName = "debug.dl";
    this->addrPort.address = QHostAddress(httpUrl.host());
    this->addrPort.port = httpUrl.port();

    QString path = httpUrl.path().mid(10);

    this->chunkID = path.toInt();
}

FileTcpBlockingDownloader::~FileTcpBlockingDownloader()
{
    // nothing
}

void FileTcpBlockingDownloader::startDownload()
{
    blockingDownloader = new BlockingDownloader(addrPort, chunkID);
    connect(blockingDownloader, SIGNAL(gotFileName(QString)),
            this, SLOT(setFileName(QString)), Qt::UniqueConnection);
    connect(blockingDownloader, SIGNAL(downloadProgress(qint64, qint64)),
            this, SIGNAL(downloadProgress(qint64,qint64)), Qt::UniqueConnection);
    connect(blockingDownloader, SIGNAL(downloaded()),
            this, SLOT(onDownloadDone()), Qt::UniqueConnection);
    blockingDownloader->start();
}

QByteArray FileTcpBlockingDownloader::downloadedData()
{
    return m_DownloadedData;
}

QString FileTcpBlockingDownloader::getFileName()
{
    return fileName;
}

void FileTcpBlockingDownloader::setFileName(QString fileName)
{
    this->fileName = fileName;
}

void FileTcpBlockingDownloader::onDownloadDone()
{
    this->m_DownloadedData = blockingDownloader->m_DownloadedData;
    blockingDownloader->exit();
    blockingDownloader->disconnect();
    blockingDownloader->deleteLater();
    emit downloaded();
}
