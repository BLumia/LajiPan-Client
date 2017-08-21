#ifndef DOWNLOADFORM_H
#define DOWNLOADFORM_H

#include "lajiutils.h"

#include <QWidget>
#include <set>
#include <mutex>

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

private:
    Ui::DownloadForm *ui;
    std::map<int, QAddressPort> fileSrvMap;
    std::set<QString> partNameSet;
    std::mutex insertDataMutex;
    QString fileName;

    void clearDownloadList();
    void refreshFileSrvMap(QTcpSocket &socket);
};

#endif // DOWNLOADFORM_H
