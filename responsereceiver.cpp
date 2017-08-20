#include "lajiutils.h"
#include "responsereceiver.h"

#include <QtEndian>
#include <QDataStream>
#include <QHostAddress>

ResponseReceiver::ResponseReceiver()
{

}

std::vector<int> ResponseReceiver::recvICdc(QTcpSocket &socket)
{
    socket.waitForReadyRead();
    QDataStream in(&socket);
    qint32 chunkCnt, chunkData;
    char recvHeader[5];
    in.readRawData(recvHeader, 4);
    std::vector<int> chunkLocationArr;
    in >> chunkCnt;
    for (int i = 1; i <= chunkCnt; i++) {
        in >> chunkData;
        chunkLocationArr.push_back(chunkData);
    }
    return chunkLocationArr;
}

ICucModel ResponseReceiver::recvICuc(QTcpSocket &socket)
{
    ICucModel result;

    socket.waitForReadyRead();
    QDataStream in(&socket);

    char recvHeader[5], recvStatus[4];
    int32_t recvSize;

    in.readRawData(recvHeader, 4);
    recvHeader[4] = '\0';
    in >> recvSize;
    in.readRawData(recvStatus, 3);
    recvStatus[3] = '\0';

    result.header = QString(recvHeader);
    result.status = QString(recvStatus);

    if (recvSize == 3) {
        return result;
    } else {
        int32_t chunkCnt, addrCnt;
        uint32_t addr, port;
        in >> chunkCnt >> addrCnt;
        result.chunkCnt = chunkCnt;
        for (int i = 1; i <= addrCnt; i++) {
            in >> addr >> port;
            QHostAddress qaddr(qFromBigEndian(addr));
            QAddressPort addrport(qaddr, port);

            result.addrPortList.push_back(addrport);
        }
        return result;
    }
}

std::map<int, QAddressPort> ResponseReceiver::recvICsr(QTcpSocket &socket)
{
    std::map<int, QAddressPort> result;
    char recvHeader[5];
    int32_t recvSize, addrCnt, srvIdx;
    uint32_t addr, port;
    socket.waitForReadyRead();
    QDataStream in(&socket);

    in.readRawData(recvHeader, 4);
    recvHeader[4] = '\0';

    in >> recvSize; // althought this is useless.
    in >> addrCnt;

    for (int i = 1; i <= addrCnt; i++) {
        in >> srvIdx >> addr >> port;

        // FIXME: port given is Updown Port, but we are using query port now.
        port = port - 61 + 80;

        QHostAddress qaddr(qFromBigEndian(addr));
        QAddressPort addrport(qaddr, port);

        result[srvIdx] = addrport;
    }

    return result;
}
