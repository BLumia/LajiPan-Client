#ifndef DOWNLOADPROGRESSITEM_H
#define DOWNLOADPROGRESSITEM_H

#include "filedownloader.h"
#include "filetcpdownloader.h"
#include "lajiutils.h"

#include <QString>
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

    FileTCPDownloader* fileDownloaderPtr = nullptr;

public slots:
    void updateDownloadProgress(qint64 downloadedSize, qint64 totalSize);
    void downloadDone();

private:
    Ui::DownloadProgressItem *ui;

signals:
    void itemDownloadDone(QString);
};

#endif // DOWNLOADPROGRESSITEM_H
