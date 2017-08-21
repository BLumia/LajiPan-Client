#include "downloadprogressitem.h"
#include "ui_downloadprogressitem.h"

#include <QDebug>
#include <QFile>

DownloadProgressItem::DownloadProgressItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadProgressItem)
{
    ui->setupUi(this);
}

DownloadProgressItem::~DownloadProgressItem()
{
    delete ui;
    if (this->fileDownloaderPtr != nullptr) delete this->fileDownloaderPtr;
}

void DownloadProgressItem::initItem(int chunkID, QAddressPort fsAddr)
{
    ui->progressBar->setValue(0);
    ui->label->setText("#" + QString::number(chunkID) + "@" + fsAddr.address.toString()
                         + ":" + QString::number(fsAddr.port));
    ui->label_2->setText("Hanging up...");
}

void DownloadProgressItem::updateDownloadProgress(qint64 downloadedSize, qint64 totalSize)
{
    if (downloadedSize > totalSize) {
        qDebug() << "What?";
        return;
    }
    ui->label_2->setText("Downloading: " + LajiUtils::humanFileSize(downloadedSize) +
                         " / " + LajiUtils::humanFileSize(totalSize));

    ui->progressBar->setMaximum(totalSize);
    ui->progressBar->setValue(downloadedSize);
}

void DownloadProgressItem::downloadDone()
{
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->label_2->setText("Download done!");

    QByteArray fileBinary(fileDownloaderPtr->downloadedData());
    QString fileName = fileDownloaderPtr->fileName;
    QFile file("Cache/" + fileName); // FIXME: download name.
    file.open(QIODevice::WriteOnly);
    file.write(fileBinary);
    file.close();
    qDebug() << "One chunk downloaded.";

    delete fileDownloaderPtr;
    fileDownloaderPtr = nullptr;

    // emit a signal for dl form?
    emit itemDownloadDone(fileName);

    return;
}
