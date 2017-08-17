#include "filelistwidget.h"
#include "lajiutils.h"
#include "requestsender.h"

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

    // TODO: check is file, not a folder.

    emit dropEventTriggered(urls);
}

void FileListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void FileListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}
