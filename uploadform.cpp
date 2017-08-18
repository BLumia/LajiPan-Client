#include "uploadform.h"
#include "ui_uploadform.h"
#include "lajiutils.h"

#include <QFileInfo>

UploadForm::UploadForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UploadForm)
{
    ui->setupUi(this);
}

UploadForm::~UploadForm()
{
    delete ui;
}

void UploadForm::updateUIText(QString filePath)
{
    QFileInfo fileInfo(filePath);
    ui->labelFileName->setText(fileInfo.fileName());
    ui->labelFilePath->setText(fileInfo.filePath());
    this->fileSize = fileInfo.size();
    ui->labelFileSize->setText(LajiUtils::humanFileSize(this->fileSize));
}

void UploadForm::updateUploadedSize(qint64 size)
{
    this->uploadedSize = size;
    if (size <= this->fileSize) {
        ui->labelUploadedSize->setText(LajiUtils::humanFileSize(size));
        float percent = size / this->fileSize;
        percent *= 100.0f;
        updateProgressbarPercent(percent);
    } else {
        qDebug() << "updateUploadedSize(): What?";
    }
}

void UploadForm::updateProgressbarPercent(float percent)
{
    ui->progressBar->setValue(percent);
}

void UploadForm::uploadDone()
{
    ui->labelUploadedSize->setText(LajiUtils::humanFileSize(this->fileSize));
    this->uploadedSize = this->fileSize;
    updateProgressbarPercent(100);
}
