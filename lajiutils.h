#ifndef LAJIUTILS_H
#define LAJIUTILS_H

#include <QByteArray>
#include <QHostAddress>
#include <QIcon>
#include <QString>
#include <QTcpSocket>

#define CHUNKSIZE_MB    64
#define CHUNKSIZE_B     67108864

class QAddressPort {

    friend QDebug operator <<(QDebug dbg, const QAddressPort &instanceObj);

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
    static QByteArray calcFf16b(QString filepath);
    static QString humanFileSize(qint64 size);
    static bool getConnectFromList(QTcpSocket &socket,
                                   std::vector<QAddressPort> &addrPortList, int number);
    static QIcon getIconByFilename(const QString &fileName);
};

#endif // LAJIUTILS_H
