#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QListWidget>
#include <QWidget>

class FileListWidget : public QListWidget
{
public:
    FileListWidget(QWidget *parent = 0);
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
};

#endif // FILELISTWIDGET_H
