#include "lajiutils.h"

#include <QCryptographicHash>
#include <QFileInfo>

QAddressPort::QAddressPort()
{
    this->address.setAddress(QHostAddress::LocalHost);
    this->port = 8061;
}

QAddressPort::QAddressPort(QHostAddress addr, int port)
{
    this->address = addr;
    this->port = port;
}

QAddressPort::QAddressPort(QString addrStr, int port)
{
    this->address.setAddress(addrStr);
    this->port = port;
}

QDebug operator <<(QDebug dbg, const QAddressPort &instanceObj)
{
    dbg.nospace() << "QAddressPort( " << instanceObj.address.toString() << " , "
                  << QString::number(instanceObj.port) << " )";
    return dbg.maybeSpace();
}

LajiUtils::LajiUtils()
{

}

QByteArray LajiUtils::calcMD5(QString filepath)
{
    QFile file(filepath);
    QCryptographicHash crypto(QCryptographicHash::Md5);

    file.open(QFile::ReadOnly);

    while(!file.atEnd()){
        crypto.addData(file.read(8192));
    }

    file.close();

    return crypto.result();
}

QByteArray LajiUtils::calcFf16b(QString filepath)
{
    QFile file(filepath);
    QByteArray ff16bByteArray(16, 0);

    file.open(QFile::ReadOnly);
    ff16bByteArray = file.peek(16);
    file.close();

    return ff16bByteArray;
}
