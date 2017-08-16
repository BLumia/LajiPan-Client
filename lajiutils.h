#ifndef LAJIUTILS_H
#define LAJIUTILS_H

#include <QByteArray>
#include <QHostAddress>
#include <QString>

class QAddressPort {
public:
    explicit QAddressPort();
    explicit QAddressPort(QHostAddress addr, int port);
    explicit QAddressPort(QString addrStr, int port);
    QHostAddress address;
    int port;
};

class LajiUtils
{
public:
    LajiUtils();
    static QByteArray calcMD5(QString filepath);
};

#endif // LAJIUTILS_H
