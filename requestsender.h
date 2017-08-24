#ifndef REQUESTSENDER_H
#define REQUESTSENDER_H

#include <QString>
#include <QTcpSocket>


class RequestSender
{
public:
    RequestSender();

    static bool sendCIfq(QTcpSocket &socket, QString fileName);
    static bool sendCIhq(QTcpSocket &socket, QString filepath);
    static bool sendCIhq(QTcpSocket &socket, QString hashStr, QString ff16bStr,
                         qint64 fileSize, QByteArray fileNameArray);
    static bool sendCFuc(QTcpSocket &socket, int32_t chunkPartID, QString hashStr,
                         qint64 chunkSize, QByteArray blob);
    static bool sendCIsr(QTcpSocket &socket);
    static bool sendCFdc(QTcpSocket &socket, int32_t chunkID);
    static bool sendPING(QTcpSocket &socket);
};

#endif // REQUESTSENDER_H
