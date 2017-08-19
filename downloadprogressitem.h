#ifndef DOWNLOADPROGRESSITEM_H
#define DOWNLOADPROGRESSITEM_H

#include "lajiutils.h"

#include <QWidget>

namespace Ui {
class DownloadProgressItem;
}

class DownloadProgressItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadProgressItem(QWidget *parent = 0);
    ~DownloadProgressItem();
    void initItem(int chunkID, QAddressPort fsAddr);

public slots:
    void updateDownloadProgress(qint64 downloadedSize, qint64 totalSize);

private:
    Ui::DownloadProgressItem *ui;
};

#endif // DOWNLOADPROGRESSITEM_H
