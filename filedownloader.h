#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QUrl httpUrl, QObject *parent = 0);
    ~FileDownloader();
    QByteArray downloadedData() /*const*/;
    QString fileName;

signals:
    void downloaded();
    void downloadProgress(qint64, qint64);

public slots:
    void emitDownloadProgress(qint64 recvSize, qint64 totalSize);

private slots:
    void fileDownloaded(QNetworkReply* pReply);
    void httpReadyRead();

private:
    QNetworkAccessManager m_WebCtrl;
    QNetworkReply *reply;
    QByteArray m_DownloadedData;
};

#endif // FILEDOWNLOADER_H
