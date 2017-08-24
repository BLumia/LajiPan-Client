#include "requestsender.h"
#include "lajiutils.h"

#include <QDataStream>
#include <QFileInfo>

RequestSender::RequestSender()
{

}

bool RequestSender::sendCIfq(QTcpSocket &socket, QString fileName)
{
    // [*CIfq*][256byte filename]
    QDataStream out(&socket);

    QByteArray fileNameArray(256, '\0');
    fileNameArray.insert(0, fileName.simplified());

    out.writeRawData("CIfq", 4);
    out.writeRawData(fileNameArray, 256);

    socket.waitForBytesWritten();

    return true;
}

bool RequestSender::sendCIhq(QTcpSocket &socket, QString filepath)
{
    QFileInfo fileinfo(filepath);
    QByteArray hash = LajiUtils::calcMD5(filepath);
    QByteArray ff16b = LajiUtils::calcFf16b(filepath);
    QByteArray fileNameArray(256, '\0');
    fileNameArray.insert(0, fileinfo.fileName().simplified());

    return sendCIhq(socket, hash.toHex(), ff16b.toHex(), fileinfo.size(), fileNameArray);
}

bool RequestSender::sendCIhq(QTcpSocket &socket, QString hashStr, QString ff16bStr,
                             qint64 fileSize, QByteArray fileNameArray)
{
    // req: [*CIhq*][hash(32bytes)][ff16b(hex, so 32bytes)][filesize(int64_t)][256byte filename]
    QDataStream out(&socket);

    socket.write("CIhq", 4);
    out.writeRawData(hashStr.toStdString().c_str(), 32);
    out.writeRawData(ff16bStr.toStdString().c_str(), 32);
    //os.writeRawData((char*)&fileSize, sizeof(int64_t));
    out << fileSize;
    out.writeRawData(fileNameArray.toStdString().c_str(), 256);
    //os.writeRawData(fileNameStr.toStdString().c_str(), 256);
    //os << fileNameArray;
    socket.waitForBytesWritten();

    return true;
}

bool RequestSender::sendCFuc(QTcpSocket &socket, int32_t chunkPartID, QString hashStr,
                             qint64 chunkSize, QByteArray blob)
{
    // req: [*CFuc*][chunk len(int64)][file hash][file part id(for client)][chunk] (token?)
    QDataStream out( &socket );
    out.writeRawData("CFuc", 4);
    out << chunkSize;
    out.writeRawData(hashStr.toStdString().c_str(), 32);
    out << chunkPartID;
    out.writeRawData(blob.toStdString().c_str(), chunkSize);

    socket.waitForBytesWritten();

    return true;
}

bool RequestSender::sendCIsr(QTcpSocket &socket)
{
    socket.write("CIsr", 4);
    socket.waitForBytesWritten();

    return true;
}

bool RequestSender::sendCFdc(QTcpSocket &socket, int32_t chunkID)
{
    QDataStream out( &socket );
    socket.write("CFdc", 4);
    out << chunkID;
    // wait?
    socket.waitForBytesWritten();
    return true;
}

bool RequestSender::sendPING(QTcpSocket &socket)
{
    socket.write("PING", 4);
    socket.waitForBytesWritten();

    return true;
}
