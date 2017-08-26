#include "blockingdownloader.h"
#include "requestsender.h"

BlockingDownloader::BlockingDownloader(QAddressPort addrPort, int32_t chunkID)
{
    this->addrPort = addrPort;
    this->chunkID = chunkID;
}

void BlockingDownloader::run()
{
    QTcpSocket socket;
    QDataStream in;
    socket.connectToHost(addrPort.address, addrPort.port);
    in.setDevice(&socket);

    if (!socket.waitForConnected()) {
        qDebug() << "BlockingDownloader::run()::waitForConnected()" << socket.errorString();
        return;
    }

    RequestSender::sendCFdc(socket, chunkID);

    qint64 totalBytes = -1;

    for(;;) {
        if (!socket.waitForReadyRead()) {
            qDebug() << "BlockingDownloader::run()::waitForReadyRead()" << socket.error() << socket.errorString();
            return;
        }

        in.startTransaction(); // Defines a restorable point

        qint64 receivedBytes = socket.bytesAvailable();
        qint64 receivedSize;
        if (receivedBytes > 16) {
            // rsp: [FCdc][chunkpartid][chunklen(int64_t)][chunk]
            if (totalBytes == -1) {
                qint32 chunkPartID;

                char recvHeader[5];
                in.readRawData(recvHeader, 4);
                in >> chunkPartID >> totalBytes;
                QString fileName = QString::number(chunkPartID) + ".dl";
                emit gotFileName(fileName);

                in.startTransaction(); // remark the start pos to blob start.
            }

            receivedSize = socket.bytesAvailable();
            emit downloadProgress(receivedSize, totalBytes);

            if (receivedSize < totalBytes) {
                in.commitTransaction();
                continue;
            } else {
                break;
            }
        } else {
            in.commitTransaction();
            continue;
        }
    }

    this->m_DownloadedData = socket.readAll();
    socket.close();
    emit downloaded();
    return;
}
