#include "filetcpdownloader.h"
#include "requestsender.h"

FileTCPDownloader::FileTCPDownloader(QUrl httpUrl, QObject *parent) :
    QObject(parent),
    tcpSocket(new QTcpSocket(this))
{
    // for some reason we still choose to write a adapter for http
    fileName = "debug.dl";
    this->addrPort.address = QHostAddress(httpUrl.host());
    this->addrPort.port = httpUrl.port();

    QString path = httpUrl.path().mid(10);

    this->chunkID = path.toInt();

    // socket signal and slots
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(sendDownloadRequest()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
}

FileTCPDownloader::~FileTCPDownloader()
{
    if (tcpSocket != nullptr) {
        tcpSocket->abort();
        delete tcpSocket;
    }
}

void FileTCPDownloader::startDownload()
{
    tcpSocket->abort();
    tcpSocket->connectToHost(addrPort.address, addrPort.port);
    in.setDevice(tcpSocket);
    // we use async approach here, so no WaitForConnection here.
    // when connected(), that will trigged a CFdc request.
}

QByteArray FileTCPDownloader::downloadedData()
{
    return m_DownloadedData;
}

void FileTCPDownloader::socketReadyRead()
{
    in.startTransaction(); // Defines a restorable point

    qint64 receivedBytes = tcpSocket->bytesAvailable();
    qint64 receivedSize;
    if (receivedBytes > 16) {
        // rsp: [FCdc][chunkpartid][chunklen(int64_t)][chunk]
        if (totalBytes == -1) {
            qint32 chunkPartID;

            char recvHeader[5];
            in.readRawData(recvHeader, 4);
            in >> chunkPartID >> totalBytes;
            this->fileName = QString::number(chunkPartID) + ".dl";
        }

        receivedSize = tcpSocket->bytesAvailable();
        emit downloadProgress(receivedSize, totalBytes);

        if (receivedSize < totalBytes) {
            in.commitTransaction();
            return;
        }
    } else {
        in.commitTransaction();
        return;
    }

    this->m_DownloadedData = tcpSocket->readAll();
    tcpSocket->close();
    //tcpSocket = nullptr;
    emit downloaded();
    return;
}

void FileTCPDownloader::sendDownloadRequest()
{
    // send CFdc
    qDebug() << "sendDownloadRequest(): TcpSocket Connected for dl chunk ID" << chunkID;
    RequestSender::sendCFdc(*tcpSocket, chunkID);
}
