#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QListWidget>
#include <QWidget>
#include <QList>
#include <QUrl>

class FileListWidget : public QListWidget
{
    Q_OBJECT
public:
    FileListWidget(QWidget *parent = 0);
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
signals:
    void dropEventTriggered(QList<QUrl> urls);
};

#endif // FILELISTWIDGET_H
