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

LajiUtils::LajiUtils()
{

}

QByteArray LajiUtils::calcMD5(QString filepath)
{
    QFileInfo fileinfo(filepath);
    QFile file(filepath);
    QCryptographicHash crypto(QCryptographicHash::Md5);

    file.open(QFile::ReadOnly);

    while(!file.atEnd()){
        crypto.addData(file.read(8192));
    }

    file.close();

    return crypto.result();
}
