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
    this->refreshFileSrvMap(socket);
    for(int& oneAddr : chunkArr) {
        int chunkFSID = oneAddr / 1000;
        int chunkID = oneAddr % 1000;
        qDebug() << "Chunk" << chunkID << "at FS" << chunkFSID << "addr:" << fileSrvMap[chunkFSID];

        isDownloading = true;

        DownloadProgressItem* dlItem = new DownloadProgressItem();
        dlItem->initItem(chunkID, fileSrvMap[chunkFSID]);
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(QSize(0,50));
        ui->downloadItemList->addItem(item);
        ui->downloadItemList->setItemWidget(item, dlItem);
    }

    return true;
}

void DownloadForm::on_closeBtn_clicked()
{
    this->clearDownloadList();
    isDownloading = false;
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
