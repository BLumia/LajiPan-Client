#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "filedownloader.h"

#include <QMainWindow>
#include <QUrl>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QHostAddress>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString updownSrvAddrPort;
    QHostAddress updownSrvAddr;
    int updownSrvPort;
    QString querySrvAddrPort;
    QHostAddress querySrvAddr;
    int querySrvPort;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
private slots:
    void on_dbgKeepAliveBtn_clicked();
    void on_dbgPingBtn_clicked();
    void on_dbgLoadFileBtn_clicked();
    void on_dbgHashQueryBtn_clicked();
    void on_dbgFillValueBtn_clicked();
    void on_dbgReportUploadBtn_clicked();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_dbgFileSrvPingBtn_clicked();
    void on_dbgSplitFileBtn_clicked();
    void on_dbgUploadChunkBtn_clicked();
    void on_dbgDownloadChunkBtn_clicked();

    // context menus:
    void refreshFileList();
    void on_listWidget_customContextMenuRequested(const QPoint &pos);
    void on_settingApplyBtn_clicked();

    // custom solts:
    void requestReceived(QNetworkReply* reply);
    void partDownloaded();

    void on_dbgFileQueryBtn_clicked();

private:
    Ui::MainWindow *ui;
    FileDownloader *downloaderHandler;
};

#endif // MAINWINDOW_H
