#ifndef OPENAGGREGATOR_H
#define OPENAGGREGATOR_H

#include <QMainWindow>
#include <QDebug>
#include <string>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlTableModel>

#include <QDialog>
#include <QInputDialog>
#include <QFileDialog>

#include <QXmlStreamReader>

#include <QStandardItemModel>

using namespace std;

namespace Ui
{
    class OpenAggregator;
}

class OpenAggregator : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit OpenAggregator(QWidget *parent = 0);
    ~OpenAggregator();
public slots:
    void AddFeed();
    void SetHost();
    void Import();
    void FolderNameChanged(QString folder);
    void DownloadFeedFinished(QNetworkReply*reply);
    void FeedOrFolderSelected(QModelIndex index);

private:
    QString mHostName;
    QSqlDatabase mDatabase;

    QStandardItemModel mModel;

    QNetworkAccessManager netMan;

    QMap<QUrl,QString> feedList;

    void ConnectToHost(const QString &iHostName);
    void RefreshFeedList();

    bool PerformQuery(const QString &query);
    bool InsertFeed(const QString &name,const QUrl &url,const QString &folder);

    Ui::OpenAggregator *ui;
};

#endif // OPENAGGREGATOR_H
