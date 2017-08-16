#include "filelistwidget.h"

#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QTcpSocket>
#include <QDebug>
#include <QUrl>

FileListWidget::FileListWidget(QWidget *parent): QListWidget(parent)
{
    this->setAcceptDrops(true); // why cant just set acceptDrops at the UI editor
}

void FileListWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;

    QString fileName = urls.first().toLocalFile();
    qDebug() << fileName;
    qDebug() << urls;

}

void FileListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void FileListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}
