#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScriptEngine>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    updownSrvAddrPort = "127.0.0.1:8061";
    querySrvAddrPort = "127.0.0.1:8080";

    if (!QDir("Cache").exists()) QDir().mkdir("Cache");
    if (!QDir("Downloaded").exists()) QDir().mkdir("Downloaded");
}

MainWindow::~MainWindow()
{
    //delete nm;
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
    Q_UNUSED(event);
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
            ui->dbgFileSrvIDEdit->text().isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Both srv IP, Port and FileSrv ID are required!");
        return;
    }
    ui->logTextBrowser->append("Test Send [FIka] to " +
                               ui->dbgSrvAddrEdit->text() + ":" +
                               ui->dbgSrvPortEdit->text());
    QTcpSocket socket;
    QHostAddress addr(ui->dbgSrvAddrEdit->text());
    int port = ui->dbgSrvPortEdit->text().toInt();
    int32_t fsId = ui->dbgFileSrvIDEdit->text().toInt();
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
    socket.write("PING", 4);
    socket.waitForBytesWritten();
    socket.waitForReadyRead();
    ui->logTextBrowser->append("Response: " + socket.readAll());
    socket.close();
}

void MainWindow::on_dbgLoadFileBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (path.isNull()) return;

    QFileInfo fileinfo(path);
    QFile file(path);
    int64_t fileSize;
    QByteArray ff16bByteArray(16, 0);

    fileSize = fileinfo.size();
    QCryptographicHash crypto(QCryptographicHash::Md5);
    file.open(QFile::ReadOnly);

    ff16bByteArray = file.peek(16);

    while(!file.atEnd()){
      crypto.addData(file.read(8192));
    }
    QByteArray hash = crypto.result();
    ui->dbgFf16bEdit->setText(ff16bByteArray.toHex());
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
    QDataStream os( &socket );

    // req: [*CIhq*][hash(32bytes)][file first 16 bytes(hex, so 32bytes)][filesize(int64_t)][256byte filename]

    QString hashStr = ui->dbgFileHashEdit->text();
    QString ff16bStr = ui->dbgFf16bEdit->text();
    QByteArray fileNameArray(256, '\0');
    qint64 fileSize = ui->dbgFileSizeEdit->text().toInt();
    QString fileNameStr = ui->dbgFileNameEdit->text();
    fileNameArray.insert(0, fileNameStr.simplified());

    socket.write("CIhq", 4);
    os.writeRawData(hashStr.toStdString().c_str(), 32);
    os.writeRawData(ff16bStr.toStdString().c_str(), 32);
    //os.writeRawData((char*)&fileSize, sizeof(int64_t));
    os << fileSize;
    os.writeRawData(fileNameArray.toStdString().c_str(), 256);
    //os.writeRawData(fileNameStr.toStdString().c_str(), 256);
    //os << fileNameArray;
    socket.waitForBytesWritten();
    socket.waitForReadyRead();

    char recvHeader[5], recvStatus[4];
    int32_t recvSize;
    os.readRawData(recvHeader, 4);
    recvHeader[4] = '\0';
    os >> recvSize;
    os.readRawData(recvStatus, 3);
    recvStatus[3] = '\0';
    if (recvSize == 3) {
        ui->logTextBrowser->append("Response: " + QString(recvHeader) +
                                   " Recv Size: " + QString::number(recvSize) +
                                   " Response State: " + QString(recvStatus));
    } else {
        int32_t chunkCnt, addrCnt;
        uint32_t addr, port;
        os >> chunkCnt >> addrCnt;
        ui->logTextBrowser->append("Response: " + QString(recvHeader) + " Recv Size: " +
                                   QString::number(recvSize) + " Chunk Cnt: " + QString::number(chunkCnt)
                                   + " Addr Cnt: " + QString::number(addrCnt));
        for (int i = 1; i <= addrCnt; i++) {
            os >> addr >> port;
            QHostAddress qaddr(addr);
            ui->logTextBrowser->append("\tChunk Addr(in uint32_t): " + qaddr.toString() +
                                        ":" + QString::number(port));
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
    this->updownSrvAddrPort = ui->srvAddrEdit->text() + ui->srvPortEdit->text();
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
            QListWidgetItem* item = new QListWidgetItem(value.toString(), ui->listWidget);;
            item->setIcon(provider.icon(QFileIconProvider::File));
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
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    QString fileName = item->text();
    QMessageBox::StandardButton btnClicked = QMessageBox::information(nullptr, "Info",
                             "Would you like to download: " + fileName + " ?",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btnClicked == QMessageBox::Yes) {
        qDebug() << "Yes Yes Yes";
    } else {
        qDebug() << "No No No";
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
    socket.write("PING", 4);
    socket.waitForBytesWritten();
    socket.waitForReadyRead();
    ui->logTextBrowser->append("Response: " + socket.readAll());
    socket.close();
}

void MainWindow::on_dbgSplitFileBtn_clicked()
{
    QString fileFullPath = ui->dbgFileNameEdit->statusTip();
    if (fileFullPath.isEmpty()) {
        QMessageBox::information(nullptr, "Info", "Please select a file by using the 'Load File' btn first.");
        return;
    }

    // TODO: split file.
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
    QDataStream os( &socket );
    QString hashStr = ui->dbgFileHashEdit->text();
    int32_t chunkPartID =  ui->dbgChunkPartIDEdit->text().toInt();
    os.writeRawData("CFuc", 4);
    os << fileSize;
    os.writeRawData(hashStr.toStdString().c_str(), 32);
    os << chunkPartID;
    os.writeRawData(blob.toStdString().c_str(), fileSize);

    socket.waitForBytesWritten();
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
    downloaderHandler = new FileDownloader(chunkUrl, this);

    connect(downloaderHandler, SIGNAL (downloaded()), this, SLOT (partDownloaded()));
}
