#include "downloadprogressitem.h"
#include "ui_downloadprogressitem.h"

#include <QDebug>

DownloadProgressItem::DownloadProgressItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadProgressItem)
{
    ui->setupUi(this);
}

DownloadProgressItem::~DownloadProgressItem()
{
    delete ui;
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
    ui->progressBar->setMaximum(totalSize);
    ui->progressBar->setValue(downloadedSize);
}
