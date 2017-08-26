#include "mainwindow.h"
#include "requestsender.h"
#include "responsereceiver.h"
#include "lajiutils.h"
#include "ui_mainwindow.h"
#include "filehttpdownloader.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QFileDialog>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QtConcurrent/QtConcurrent>
#include <QMenu>
#include <vector>
#include <QFuture>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    uploadFormPtr = new UploadForm(0);
    downloadFormPtr = new DownloadForm(0);

    md5resultWatcher = new QFutureWatcher<QByteArray>();
    statusProgressBar = new QProgressBar();

    ui->statusBar->addPermanentWidget(statusProgressBar);
    statusProgressBar->hide();

    updownSrvAddrPort = "127.0.0.1:8061";
    querySrvAddrPort = "127.0.0.1:8080";
    querySrvAddr.setAddress("127.0.0.1");
    updownSrvAddr.setAddress("127.0.0.1");
    querySrvPort = 8080;
    updownSrvPort = 8061;

    ui->listWidget->setAcceptDrops(true);

    if (!QDir("Cache").exists()) QDir().mkdir("Cache");
    if (!QDir("Downloaded").exists()) QDir().mkdir("Downloaded");

    connect(md5resultWatcher, SIGNAL(finished()),
            this, SLOT(doUploadFile()), Qt::UniqueConnection);
    connect(ui->listWidget,SIGNAL(dropEventTriggered(QList<QUrl>)),
            this,SLOT(on_listWidget_dropEventTriggered(QList<QUrl>)), Qt::UniqueConnection);
    //connect(statusProgressBar, SIGNAL())
}

MainWindow::~MainWindow()
{
    //delete nm;
    delete statusProgressBar;
    delete ui;
    delete uploadFormPtr;
    delete downloadFormPtr;
    delete md5resultWatcher;

    if (downloaderHandler != nullptr) delete downloaderHandler;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
    Q_UNUSED(event);
//    downloadFormPtr->show();
//    downloadFormPtr->update();
//    qDebug() << "triggered";
/*
    QCursor cur=this->cursor();
    QMenu fileListViewCtxMenu;
    fileListViewCtxMenu.addAction("Refresh" , this, SLOT(refreshFileList()));
    fileListViewCtxMenu.exec(cur.pos());
*/
}

void MainWindow::on_dbgKeepAliveBtn_clicked()
{
    if (ui->dbgSrvAddrEdit->text().isEmpty() ||
            ui->dbgSrvPortEdit->text().isEmpty() ||
            ui->dbgFileSrvUpdownPortEdit->text().isEmpty() ||
            ui->dbgFileSrvIDEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Both srv IP, Port, FileSrv Updown Port"
                                                  " and FileSrv ID are required!");
        return;
    }
    ui->logTextBrowser->append("Test Send [FIka] to " +
                               ui->dbgSrvAddrEdit->text() + ":" +
                               ui->dbgSrvPortEdit->text());
    QTcpSocket socket;
    QHostAddress addr(ui->dbgSrvAddrEdit->text());
    int port = ui->dbgSrvPortEdit->text().toInt();
    int32_t fsId = ui->dbgFileSrvIDEdit->text().toInt();
    int32_t fsPort = ui->dbgFileSrvUpdownPortEdit->text().toInt();
    socket.connectToHost(addr, port);
    // will block. should in another thread
    if (!socket.waitForConnected()) {
        ui->logTextBrowser->append(socket.errorString());
        return;
    }
    QDataStream os( &socket );
    socket.write("FIka", 4);
    //socket.write((char*)&fsId, sizeof(int32_t));
    os << fsId;
    os << fsPort;
    socket.waitForBytesWritten();
    ui->logTextBrowser->append("Sent a Keep-Alive!");
    socket.close();
}

void MainWindow::on_dbgPingBtn_clicked()
{
    if (ui->dbgSrvAddrEdit->text().isEmpty() ||
            ui->dbgSrvPortEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Both srv IP and Port are required!");
        return;
    }
    ui->logTextBrowser->append("Test Send [PING] to " +
                               ui->dbgSrvAddrEdit->text() + ":" +
                               ui->dbgSrvPortEdit->text());
    QTcpSocket socket;
    QHostAddress addr(ui->dbgSrvAddrEdit->text());
    int port = ui->dbgSrvPortEdit->text().toInt();
    socket.connectToHost(addr, port);
    // will block. should in another thread
    if (!socket.waitForConnected()) {
        //emit error(socket.error(), socket.errorString());
        ui->logTextBrowser->append(socket.errorString());
        return;
    }
    RequestSender::sendPING(socket);
    socket.waitForReadyRead();
    ui->logTextBrowser->append("Response: " + socket.readAll());
    socket.close();
}

void MainWindow::on_dbgLoadFileBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (path.isNull()) return;

    QFileInfo fileinfo(path);
    int64_t fileSize = fileinfo.size();
    QByteArray hash = LajiUtils::calcMD5(path);
    QByteArray ff16b = LajiUtils::calcFf16b(path);

    ui->dbgFf16bEdit->setText(ff16b.toHex());
    ui->dbgFileHashEdit->setText(hash.toHex());
    ui->dbgFileNameEdit->setText(fileinfo.fileName());
    ui->dbgFileNameEdit->setStatusTip(path);
    ui->dbgFileSizeEdit->setText(tr("%1").arg(fileSize));
}

void MainWindow::on_dbgHashQueryBtn_clicked()
{
    if (ui->dbgSrvAddrEdit->text().isEmpty() ||
            ui->dbgSrvPortEdit->text().isEmpty() ||
            ui->dbgFf16bEdit->text().isEmpty() ||
            ui->dbgFileHashEdit->text().isEmpty() ||
            ui->dbgFileNameEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Srv IP, Port, Ff16b, FileHash, "
                                                  "FileName are required!");
        return;
    }
    ui->logTextBrowser->append("Test Send [CIhq] to " +
                               ui->dbgSrvAddrEdit->text() + ":" +
                               ui->dbgSrvPortEdit->text());
    QTcpSocket socket;
    QHostAddress addr(ui->dbgSrvAddrEdit->text());
    int port = ui->dbgSrvPortEdit->text().toInt();
    socket.connectToHost(addr, port);
    // will block. should in another thread
    if (!socket.waitForConnected()) {
        ui->logTextBrowser->append(socket.errorString());
        return;
    }

    QString hashStr = ui->dbgFileHashEdit->text();
    QString ff16bStr = ui->dbgFf16bEdit->text();
    QByteArray fileNameArray(256, '\0');
    qint64 fileSize = ui->dbgFileSizeEdit->text().toInt();
    QString fileNameStr = ui->dbgFileNameEdit->text();
    fileNameArray.insert(0, fileNameStr.simplified());

    RequestSender::sendCIhq(socket, hashStr, ff16bStr, fileSize, fileNameArray);
    ICucModel receivedData = ResponseReceiver::recvICuc(socket);

    if (receivedData.status.compare("404") != 0) {
        ui->logTextBrowser->append("Response: " + receivedData.header +
                                   " Response State: " + receivedData.status);
    } else {
        int addrCnt = receivedData.addrPortList.size();
        ui->logTextBrowser->append("Response: " + receivedData.header +
                                   " Chunk Cnt: " + QString::number(receivedData.chunkCnt)
                                   + " Addr Cnt: " + QString::number(addrCnt));
        for (QAddressPort& oneAddrPort : receivedData.addrPortList) {
            ui->logTextBrowser->append("\tChunk Addr(in uint32_t): " + oneAddrPort.address.toString() +
                                        ":" + QString::number(oneAddrPort.port));
        }
    }


    socket.close();
}

void MainWindow::on_dbgFillValueBtn_clicked()
{
    ui->dbgSrvAddrEdit->setText("127.0.0.1");
    ui->dbgSrvPortEdit->setText("8061");
    ui->dbgFileSrvAddrEdit->setText("127.0.0.1");
    ui->dbgFileSrvPortEdit->setText("7061");
    ui->dbgFileSrvQueryPortEdit->setText("7080");
    ui->dbgFileSrvUpdownPortEdit->setText("7061");
}

void MainWindow::on_dbgReportUploadBtn_clicked()
{
    if (ui->dbgSrvAddrEdit->text().isEmpty() ||
            ui->dbgSrvPortEdit->text().isEmpty() ||
            ui->dbgFileSrvIDEdit->text().isEmpty() ||
            ui->dbgFileHashEdit->text().isEmpty() ||
            ui->dbgChunkIDEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Srv IP, Port, FileSrvID, ChunkID and FileHash"
                                                  " are required!");
        return;
    }

    QTcpSocket socket;
    QHostAddress addr(ui->dbgSrvAddrEdit->text());
    int port = ui->dbgSrvPortEdit->text().toInt();
    socket.connectToHost(addr, port);
    // will block. should in another thread
    if (!socket.waitForConnected()) {
        //emit error(socket.error(), socket.errorString());
        ui->logTextBrowser->append(socket.errorString());
        return;
    }
    QDataStream os( &socket );
    QString hashBuffer = ui->dbgFileHashEdit->text();
    socket.write("FIru", 4);
    os.writeRawData(hashBuffer.toStdString().c_str(), 32);
    qint32 chunkID = ui->dbgChunkIDEdit->text().toInt();
    qint32 fsID = ui->dbgFileSrvIDEdit->text().toInt();
    os << chunkID;
    os << fsID;

    socket.waitForBytesWritten();
    ui->logTextBrowser->append("Sent a [FIru] uploaded info.");
    socket.close();
}

void MainWindow::refreshFileList()
{
    ui->listWidget->clear();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(requestReceived(QNetworkReply*)));
    manager->get(QNetworkRequest("http://" + querySrvAddrPort + "/file"));
}

void MainWindow::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    QCursor cur=this->cursor();
    QMenu fileListViewCtxMenu;
    fileListViewCtxMenu.addAction("Refresh" , this, SLOT(refreshFileList()));
    fileListViewCtxMenu.exec(cur.pos());
}

void MainWindow::on_settingApplyBtn_clicked()
{
    this->querySrvAddrPort = ui->querySrvAddrEdit->text() + ui->querySrvPortEdit->text();
    this->querySrvAddr.setAddress(ui->querySrvAddrEdit->text());
    this->querySrvPort = ui->querySrvPortEdit->text().toInt();
    this->updownSrvAddrPort = ui->srvAddrEdit->text() + ui->srvPortEdit->text();
    this->updownSrvAddr.setAddress(ui->srvAddrEdit->text());
    this->updownSrvPort = ui->srvPortEdit->text().toInt();
}

void MainWindow::requestReceived(QNetworkReply *reply)
{
    reply->deleteLater();

    if(reply->error() == QNetworkReply::NoError) {
        QString data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
        QJsonObject rootObj = doc.object();
        QJsonArray fileList = rootObj["filelist"].toArray();
        QFileIconProvider provider;
        foreach (const QJsonValue & value, fileList) {
            QListWidgetItem* item = new QListWidgetItem(value.toString(), ui->listWidget);
            item->setIcon(provider.icon(QFileIconProvider::File));
            item->setIcon(LajiUtils::getIconByFilename(value.toString()));
            ui->listWidget->addItem(item);
        }
    } else {
        QMessageBox::information(nullptr, "Error!", "Something wrong with response: " +
                                                  reply->errorString());
        ui->logTextBrowser->append("Request received error: " + reply->errorString() +
                                   " Server Addr: " + querySrvAddrPort);
    }
}

void MainWindow::partDownloaded()
{
    QByteArray fileBinary(downloaderHandler->downloadedData());
    QFile file("Downloaded/debug.dl");
    file.open(QIODevice::WriteOnly);
    file.write(fileBinary);
    file.close();
    ui->logTextBrowser->append("Test download done.");
    QFileInfo fileinfo("Downloaded/debug.dl");
    QDesktopServices::openUrl( fileinfo.absolutePath() );

    downloaderHandler->disconnect();
    delete downloaderHandler;
    downloaderHandler = nullptr;
}

void MainWindow::on_listWidget_dropEventTriggered(QList<QUrl> urls)
{
    // TODO: warn user if already uploading file.
    // set m_tobeUploadedFileName to "*" (filename can't contain '*') means not uploading.
    QString filePath = urls.first().toLocalFile();

    qDebug() << filePath;
    //qDebug() << urls;
    m_tobeUploadedFilePath = filePath;

    QFuture<QByteArray> future = QtConcurrent::run(LajiUtils::calcMD5, filePath);
    md5resultWatcher->setFuture(future);

    ui->statusBar->showMessage(tr("Calculating MD5..."));
    statusProgressBar->show();
    statusProgressBar->setRange(0,0);
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString fileName = item->text();

    QTcpSocket socket;
    socket.connectToHost(updownSrvAddr, updownSrvPort);
    RequestSender::sendCIfq(socket, fileName);
    std::vector<int> chunkLocationArr( ResponseReceiver::recvICdc(socket) );

    if (chunkLocationArr.size() == 0) {
        QMessageBox::information(nullptr, "Warning",
                                 "Seems `" + fileName + "` is not ready for download.");
    } else {
        QMessageBox::StandardButton btnClicked = QMessageBox::information(nullptr, "Info",
                                 "Would you like to download: " + fileName + " ?",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btnClicked == QMessageBox::Yes) {
            qDebug() << "Yes Yes Yes";
            qDebug() << chunkLocationArr;
            downloadFormPtr->show();
            downloadFormPtr->update();
            downloadFormPtr->doDownload(socket, fileName, chunkLocationArr);

        } else {
            qDebug() << "No No No";
        }
    }
}

void MainWindow::on_dbgFileSrvPingBtn_clicked()
{
    if (ui->dbgFileSrvAddrEdit->text().isEmpty() ||
            ui->dbgFileSrvPortEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Both FS srv IP and Port are required!");
        return;
    }
    ui->logTextBrowser->append("Test Send [PING] to " +
                               ui->dbgFileSrvAddrEdit->text() + ":" +
                               ui->dbgFileSrvPortEdit->text());
    QTcpSocket socket;
    QHostAddress addr(ui->dbgFileSrvAddrEdit->text());
    int port = ui->dbgFileSrvPortEdit->text().toInt();
    socket.connectToHost(addr, port);
    // will block. should in another thread
    if (!socket.waitForConnected()) {
        //emit error(socket.error(), socket.errorString());
        ui->logTextBrowser->append(socket.errorString());
        return;
    }
    RequestSender::sendPING(socket);
    socket.waitForReadyRead();
    ui->logTextBrowser->append("Response: " + socket.readAll());
    socket.close();
}

void MainWindow::on_dbgUploadChunkBtn_clicked()
{
    QString fileFullPath = ui->dbgFileNameEdit->statusTip();
    if (fileFullPath.isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Please select a file by using the 'Load File' btn first.");
        return;
    }
    if (ui->dbgFileSrvAddrEdit->text().isEmpty() ||
            ui->dbgFileSrvPortEdit->text().isEmpty() ||
            ui->dbgFileHashEdit->text().isEmpty() ||
            ui->dbgChunkPartIDEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Both FS srv IP, Port , File hash and Chunk Part ID are required!");
        return;
    }

    ui->logTextBrowser->append("Test Send [CFuc] to " +
                               ui->dbgFileSrvAddrEdit->text() + ":" +
                               ui->dbgFileSrvPortEdit->text());

    QFile willUpload(fileFullPath);
    QFileInfo fileinfo(fileFullPath);
    if (!willUpload.open(QIODevice::ReadOnly)) {
        QMessageBox::information(nullptr, "Info", "Error when opening file!");
        return;
    }
    QByteArray blob = willUpload.readAll();
    qint64 fileSize = fileinfo.size();
    willUpload.close();

    QTcpSocket socket;
    QHostAddress addr(ui->dbgFileSrvAddrEdit->text());
    int port = ui->dbgFileSrvPortEdit->text().toInt();
    socket.connectToHost(addr, port);
    // will block. should in another thread
    if (!socket.waitForConnected()) {
        ui->logTextBrowser->append(socket.errorString());
        return;
    }
    // req: [*CFuc*][chunk len(int64)][file hash][file part id(for client)][chunk] (token?)
    QString hashStr = ui->dbgFileHashEdit->text();
    int32_t chunkPartID =  ui->dbgChunkPartIDEdit->text().toInt();

    RequestSender::sendCFuc(socket, chunkPartID, hashStr, fileSize, blob);

    socket.waitForReadyRead();

    ui->logTextBrowser->append("Response: " + socket.readAll());

    socket.close();
    return;
}

void MainWindow::on_dbgDownloadChunkBtn_clicked()
{
    if (ui->dbgFileSrvAddrEdit->text().isEmpty() ||
            ui->dbgFileSrvQueryPortEdit->text().isEmpty() ||
            ui->dbgChunkIDEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Both FS srv IP, QueryPort and Chunk ID are required!");
        return;
    }

    QUrl chunkUrl("http://" + ui->dbgFileSrvAddrEdit->text() + ':' +
                  ui->dbgFileSrvQueryPortEdit->text() + "/download/" +
                  ui->dbgChunkIDEdit->text());
    downloaderHandler = new FileHTTPDownloader(chunkUrl, this);

    connect(downloaderHandler, SIGNAL (downloaded()), this, SLOT (partDownloaded()), Qt::UniqueConnection);
    connect(downloaderHandler, SIGNAL (downloadProgress(qint64, qint64)),
            this, SLOT (updateDbgDownloadProgress(qint64, qint64)), Qt::UniqueConnection);

    downloaderHandler->startDownload();
}

void MainWindow::on_dbgFileQueryBtn_clicked()
{
    if (ui->dbgSrvAddrEdit->text().isEmpty() ||
            ui->dbgSrvPortEdit->text().isEmpty() ||
            ui->dbgFilePathEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Both Srv IP, QueryPort and Chunk ID are required!");
        return;
    }

    QTcpSocket socket;
    QHostAddress addr(ui->dbgSrvAddrEdit->text());
    int port = ui->dbgSrvPortEdit->text().toInt();
    socket.connectToHost(addr, port);

    if (!socket.waitForConnected()) {
        QMessageBox::information(nullptr, "Connection failed!",
                                 "Detail: " + socket.errorString());
        return;
    }

    RequestSender::sendCIfq(socket, ui->dbgFilePathEdit->text());
    std::vector<int> chunkLocationArr( ResponseReceiver::recvICdc(socket) );

    qDebug() << chunkLocationArr;

    ui->logTextBrowser->append("Response: [ICdc] ChunkCnt: " + QString::number(chunkLocationArr.size()));
    socket.close();
}

void MainWindow::updateDbgDownloadProgress(qint64 downloadedSize, qint64 totalSize)
{
    if (downloadedSize < 0) qDebug() << "updateDbgDownloadProgress(): ???";
    ui->dbgDlProgressBar->setMaximum(totalSize);
    ui->dbgDlProgressBar->setValue(downloadedSize);
}

void MainWindow::on_dbgSrvStatRequestBtn_clicked()
{
    if (ui->dbgSrvAddrEdit->text().isEmpty() ||
            ui->dbgSrvPortEdit->text().isEmpty() ) {
        QMessageBox::information(nullptr, "Info", "Both Srv IP and Port are required!");
        return;
    }

    QTcpSocket socket;
    QHostAddress addr(ui->dbgSrvAddrEdit->text());
    int port = ui->dbgSrvPortEdit->text().toInt();
    socket.connectToHost(addr, port);

    if (!socket.waitForConnected()) {
        QMessageBox::information(nullptr, "Connection failed!",
                                 "Detail: " + socket.errorString());
        return;
    }

    RequestSender::sendCIsr(socket);
    std::map<int, QAddressPort> result = ResponseReceiver::recvICsr(socket);
    qDebug() << result;
}

void MainWindow::onUpdateUploadProgress(qint64 uploadedSize, qint64 totalSize)
{
    float percent = uploadedSize / (float)totalSize * 100.0f;
    uploadFormPtr->updateUploadedSize(uploadedSize);
    statusProgressBar->setValue(percent);
}

void MainWindow::onUploadDone()
{
    uploadFormPtr->uploadDone();
    statusProgressBar->hide();

    // Done.
    ui->statusBar->clearMessage();
    ui->statusBar->showMessage(tr("Upload Done!"), 2000);
    QMessageBox::information(nullptr, "Info", "Upload done! Will refresh the list for ya.");
    this->refreshFileList();

    fileUploader->exit();
    fileUploader->disconnect();
    fileUploader->deleteLater(); // ??
}

// trigged after MD5 sum get result.
void MainWindow::doUploadFile()
{
    ui->statusBar->showMessage(tr("Uploading..."));
    statusProgressBar->setMaximum(100);

    QTcpSocket socket;
    QString filePath = m_tobeUploadedFilePath;

    socket.connectToHost(updownSrvAddr, updownSrvPort);
    if (!socket.waitForConnected()) {
        //emit error(socket.error(), socket.errorString());
        QMessageBox::information(nullptr, "Connection failed!",
                                 "Refer to setting tab and update the informations.\nDetail: "
                                 + socket.errorString());
        return;
    }

    QFileInfo fileinfo(filePath);
    QByteArray md5bin = md5resultWatcher->result();
    QString md5hexStr = md5bin.toHex();
    QString ff16bStr = LajiUtils::calcFf16b(filePath).toHex();
    QByteArray fileNameArray(256, '\0');
    fileNameArray.insert(0, fileinfo.fileName().simplified());

    RequestSender::sendCIhq(socket, md5hexStr, ff16bStr, fileinfo.size(), fileNameArray);
    ICucModel receivedData = ResponseReceiver::recvICuc(socket);

    qDebug() << receivedData.status << receivedData.addrPortList << md5hexStr;

    if (receivedData.status.compare(QString("200")) == 0) {
        // flash upload.
        ui->statusBar->clearMessage();
        ui->statusBar->showMessage(tr("Flash Upload Done!"), 2000);
        statusProgressBar->hide();
        QMessageBox::information(nullptr, "Info", "Flash upload done! Will refresh the list for ya.");
        this->refreshFileList();
        return;
    }

    if (receivedData.status.compare(QString("403")) == 0) {
        // flash upload.
        ui->statusBar->clearMessage();
        statusProgressBar->hide();
        QMessageBox::warning(nullptr, "Warning", "but we wont trigged this dialog at all.");
        this->refreshFileList();
        return;
    }

    uploadFormPtr->updateUIText(filePath);
    uploadFormPtr->updateUploadedSize(0);

    fileUploader = new FileUploader(QAddressPort(updownSrvAddr, updownSrvPort), filePath, md5hexStr, receivedData);
    connect(fileUploader, SIGNAL(uploadProgress(qint64,qint64)),
            this, SLOT(onUpdateUploadProgress(qint64,qint64)), Qt::UniqueConnection);
    connect(fileUploader, SIGNAL(uploaded()), this, SLOT(onUploadDone()), Qt::UniqueConnection);
    fileUploader->start();

}
