#include "lajiutils.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QApplication>
#include <QStyle>

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

QString LajiUtils::humanFileSize(qint64 size)
{
    double num = size;
    QStringList list;
    list << "KiB" << "MiB" << "GiB" << "TiB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1024.0 && i.hasNext()) {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num,'f',2)+" "+unit;
}

bool LajiUtils::getConnectFromList(QTcpSocket &socket,
                                   std::vector<QAddressPort> &addrPortList, int number)
{
    int addrListSize = addrPortList.size();
    if (addrListSize == 0) return false;

    number = number % addrListSize;
    QAddressPort qap = addrPortList[number];

    socket.close();
    socket.connectToHost(qap.address, qap.port);

    if (!socket.waitForConnected()) {
        // improvement avaliable: try next addr if possible
        qDebug() << "getConnectFromList(): " << socket.errorString();
        return false;
    }

    return true;
}

QIcon LajiUtils::getIconByFilename(const QString &fileName)
{
    QMimeDatabase mime_database;

    QIcon icon;
    QList<QMimeType> mime_types = mime_database.mimeTypesForFileName(fileName);
    for (int i=0; i < mime_types.count() && icon.isNull(); i++) {
        icon = QIcon::fromTheme(mime_types[i].iconName());
    }

    if (icon.isNull()) {
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    } else {
        return icon;
    }

}
