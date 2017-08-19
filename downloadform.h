#ifndef DOWNLOADFORM_H
#define DOWNLOADFORM_H

#include "lajiutils.h"

#include <QWidget>

namespace Ui {
class DownloadForm;
}

class DownloadForm : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadForm(QWidget *parent = 0);
    ~DownloadForm();

    bool doDownload(QTcpSocket &socket, std::vector<int> &chunkArr);
    bool isDownloading = false;

private slots:
    void on_closeBtn_clicked();

private:
    Ui::DownloadForm *ui;
    std::map<int, QAddressPort> fileSrvMap;

    void clearDownloadList();
    void refreshFileSrvMap(QTcpSocket &socket);
};

#endif // DOWNLOADFORM_H
