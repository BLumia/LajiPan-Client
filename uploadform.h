#ifndef UPLOADFORM_H
#define UPLOADFORM_H

#include <QWidget>

namespace Ui {
class UploadForm;
}

class UploadForm : public QWidget
{
    Q_OBJECT

public:
    explicit UploadForm(QWidget *parent = 0);
    ~UploadForm();
    void updateUIText(QString filePath);
    void updateUploadedSize(qint64 size);
    void uploadDone();

private:
    qint64 fileSize;
    qint64 uploadedSize;
    void updateProgressbarPercent(float percent);
    Ui::UploadForm *ui;
};

#endif // UPLOADFORM_H
