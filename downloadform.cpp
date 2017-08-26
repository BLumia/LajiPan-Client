#include "downloadform.h"
#include "downloadprogressitem.h"
#include "requestsender.h"
#include "responsereceiver.h"
#include "ui_downloadform.h"

#include <QFile>
#include <QMessageBox>
#include <QDesktopServices>
#include <QtConcurrent/QtConcurrent>

DownloadForm::DownloadForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadForm)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    this->fileName = "unset";
}

DownloadForm::~DownloadForm()
{
    delete ui;
}

bool DownloadForm::doDownload(QTcpSocket &socket, QString fileName, std::vector<int> &chunkArr)
{
    this->partNameSet.clear();
    this->refreshFileSrvMap(socket);
    socket.close(); // no longer need this
    this->fileName = fileName;
    ui->progressBar->setValue(0);
    int queueSize = 3;
    FileDownloader* fdseats[3];
    int seatUsed = 0;
    for(int& oneAddr : chunkArr) {
        int chunkFSID = oneAddr / 1000;
        int chunkID = oneAddr % 1000;
        qDebug() << "Chunk" << chunkID << "at FS" << chunkFSID << "addr:" << fileSrvMap[chunkFSID];

        isDownloading = true;

        QAddressPort &addrport = fileSrvMap[chunkFSID];
        DownloadProgressItem* dlItem = new DownloadProgressItem();
        dlItem->initItem(chunkID, fileSrvMap[chunkFSID]);

        QUrl chunkUrl("http://" + addrport.address.toString() + ':' +
                          QString::number(addrport.port) + "/download/" +
                          QString::number(chunkID));
        dlItem->fileDownloaderPtr = new FileTCPDownloader(chunkUrl, dlItem);

        // conn
        connect(dlItem->fileDownloaderPtr, SIGNAL (downloaded()),
                dlItem, SLOT (downloadDone()), Qt::UniqueConnection);
        connect(dlItem->fileDownloaderPtr, SIGNAL (downloadProgress(qint64, qint64)),
                dlItem, SLOT (updateDownloadProgress(qint64, qint64)), Qt::UniqueConnection);
        connect(dlItem, SIGNAL(itemDownloadDone(QString)),
                this, SLOT(checkDownloadDone(QString)), Qt::UniqueConnection);

        if (queueSize > 0) {
            fdseats[seatUsed] = dlItem->fileDownloaderPtr;
            queueSize--;
            seatUsed++;
        } else {
            this->downloadQueue.push( dlItem->fileDownloaderPtr );
        }

        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(QSize(0,50));
        ui->downloadItemList->addItem(item);
        ui->downloadItemList->setItemWidget(item, dlItem);
    }
    ui->progressBar->setValue(5);

    for(int i = 0; i < seatUsed; i++) {
        fdseats[i]->startDownload();
    }

    return true;
}

void DownloadForm::on_closeBtn_clicked()
{
    this->clearDownloadList();
    isDownloading = false;
    this->hide();
}

void DownloadForm::checkDownloadDone(QString partName)
{
    // mutex guard?
    std::lock_guard<std::mutex> guardian(insertDataMutex);
    this->partNameSet.insert(partName);
    if (this->partNameSet.size() == ui->downloadItemList->count()) {
        //download done!
        ui->progressBar->setValue(95);
        qDebug() << "Download DONE!!! combining files...";
        QFile file("Downloaded/" + this->fileName);
        file.open(QIODevice::WriteOnly);
        QDataStream out(&file);

        int chunkCnt = ui->downloadItemList->count();
        for(int i = 1; i <= chunkCnt; i++) {
            QFile readChunkFile("Cache/" + QString::number(i) + ".dl");
            if (!readChunkFile.open(QIODevice::ReadOnly)) {
                QMessageBox::information(nullptr, "Info", "Error when processing file!");
                return;
            }
            QByteArray blob = readChunkFile.readAll();
            readChunkFile.close();
            out.writeRawData(blob, blob.size());
        }

        file.close();
        ui->progressBar->setValue(100);
    } else {
        float done = this->partNameSet.size();
        float total = ui->downloadItemList->count();

        if (this->downloadQueue.empty() == false) {
            FileDownloader* ptr = this->downloadQueue.front();
            ptr->startDownload();
            downloadQueue.pop();
        }

        int progressbarval = done / total * 90.0;
        ui->progressBar->setValue(progressbarval);
    }
}

void DownloadForm::clearDownloadList()
{
    while(ui->downloadItemList->count() >0) {
        QListWidgetItem* itemPtr = ui->downloadItemList->takeItem(0);
        delete ui->downloadItemList->itemWidget(itemPtr);
        delete itemPtr;
    }
}

void DownloadForm::refreshFileSrvMap(QTcpSocket &socket)
{
    RequestSender::sendCIsr(socket);
    this->fileSrvMap.clear();
    this->fileSrvMap = ResponseReceiver::recvICsr(socket);
}

void DownloadForm::on_pushButton_clicked()
{
    QDesktopServices::openUrl( QDir("./Downloaded/").absolutePath() );
}
