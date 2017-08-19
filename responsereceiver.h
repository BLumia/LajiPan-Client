#ifndef RESPONSERECEIVER_H
#define RESPONSERECEIVER_H

#include "icucmodel.h"

#include <QTcpSocket>
#include <vector>

class ResponseReceiver
{
public:
    ResponseReceiver();
    static std::vector<int>             recvICdc(QTcpSocket &socket);
    static ICucModel                    recvICuc(QTcpSocket &socket);
    static std::map<int, QAddressPort>  recvICsr(QTcpSocket &socket);
};

#endif // RESPONSERECEIVER_H
