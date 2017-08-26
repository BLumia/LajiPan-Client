#include "fileuploader.h"
#include "icucmodel.h"
#include "requestsender.h"

#include <QFileInfo>
#include <QTcpSocket>

FileUploader::FileUploader(QAddressPort addrPort, QString filePath, QString &md5Str, ICucModel &icucData)
{
    this->addrPort = addrPort;
    this->filePath = filePath;
    this->md5Str = md5Str;
    this->icucData = icucData;
}

void FileUploader::run()
{
    QFile file(filePath);
    qint64 uploadedSize = 0;
    qint64 fileSize = file.size();
    file.open(QFile::ReadOnly);

    QTcpSocket socket;

    socket.connectToHost(addrPort.address, addrPort.port);
    if (!socket.waitForConnected()) {
        //emit error(socket.error(), socket.errorString());
        qDebug() << "FileUploader::run() connectToHost:" << socket.errorString();
        return;
    }

    // Do upload:
    emit uploadProgress(uploadedSize, fileSize);
    // also a speed(QString) argu?

    int chunkPartID = 1;
    while(!file.atEnd()){
        QByteArray blob = file.read(CHUNKSIZE_B);
        LajiUtils::getConnectFromList(socket, icucData.addrPortList, chunkPartID);
        RequestSender::sendCFuc(socket, chunkPartID, md5Str, blob.size(), blob);
        socket.waitForReadyRead();
        uploadedSize += blob.size();
        emit uploadProgress(uploadedSize, fileSize);
        qDebug() << "Drop file upload: Chunk #" << QString::number(chunkPartID)
                                   << " Status: " <<  socket.readAll();
        chunkPartID++;
    }

    emit uploaded();
}
