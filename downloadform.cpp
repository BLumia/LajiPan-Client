#include "downloadform.h"
#include "downloadprogressitem.h"
#include "requestsender.h"
#include "responsereceiver.h"
#include "ui_downloadform.h"

DownloadForm::DownloadForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadForm)
{
    ui->setupUi(this);
}

DownloadForm::~DownloadForm()
{
    delete ui;
}

bool DownloadForm::doDownload(QTcpSocket &socket, std::vector<int> &chunkArr)
{
    this->partNameSet.clear();
    this->refreshFileSrvMap(socket);
    ui->progressBar->setValue(0);
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
        dlItem->fileDownloaderPtr = new FileDownloader(chunkUrl, dlItem);

        // conn
        connect(dlItem->fileDownloaderPtr, SIGNAL (downloaded()),
                dlItem, SLOT (downloadDone()), Qt::UniqueConnection);
        connect(dlItem->fileDownloaderPtr, SIGNAL (downloadProgress(qint64, qint64)),
                dlItem, SLOT (updateDownloadProgress(qint64, qint64)), Qt::UniqueConnection);
        connect(dlItem, SIGNAL(itemDownloadDone(QString)),
                this, SLOT(checkDownloadDone(QString)), Qt::UniqueConnection);

        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(QSize(0,50));
        ui->downloadItemList->addItem(item);
        ui->downloadItemList->setItemWidget(item, dlItem);
    }
    ui->progressBar->setValue(3);

    return true;
}

void DownloadForm::on_closeBtn_clicked()
{
    this->clearDownloadList();
    isDownloading = false;
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
    } else {
        float done = this->partNameSet.size();
        float total = ui->downloadItemList->count();
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
