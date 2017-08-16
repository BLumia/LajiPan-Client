#ifndef ICUCMODEL_H
#define ICUCMODEL_H

#include "lajiutils.h"

#include <QString>



class ICucModel
{
public:
    QString header;
    QString status;
    qint32 chunkCnt = 0;
    std::vector<QAddressPort> addrPortList;

    ICucModel();
};

#endif // ICUCMODEL_H
