#ifndef DOWNLOADFORM_H
#define DOWNLOADFORM_H

#include "filedownloader.h"
#include "filetcpdownloader.h"
#include "lajiutils.h"

#include <QWidget>
#include <set>
#include <mutex>
#include <queue>

namespace Ui {
class DownloadForm;
}

class DownloadForm : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadForm(QWidget *parent = 0);
    ~DownloadForm();

    bool doDownload(QTcpSocket &socket, QString fileName, std::vector<int> &chunkArr);
    bool isDownloading = false;

private slots:
    void on_closeBtn_clicked();
    void checkDownloadDone(QString partName);

    void on_pushButton_clicked();

private:
    Ui::DownloadForm *ui;
    std::map<int, QAddressPort> fileSrvMap;
    std::set<QString> partNameSet;
    std::mutex insertDataMutex;
    QString fileName;
    std::queue<FileDownloader*> downloadQueue;

    void clearDownloadList();
    void refreshFileSrvMap(QTcpSocket &socket);
};

#endif // DOWNLOADFORM_H
