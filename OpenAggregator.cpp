#include "OpenAggregator.h"
#include "ui_OpenAggregator.h"

#define FEEDTABLE_LOCATION_URL      0
#define FEEDTABLE_LOCATION_NAME     1
#define FEEDTABLE_LOCATION_FOLDER   2

OpenAggregator::OpenAggregator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OpenAggregator)
{
    ui->setupUi(this);
}

OpenAggregator::~OpenAggregator()
{
    if(mDatabase.isOpen())
        mDatabase.close();
    delete ui;
}

//CONNECT

void OpenAggregator::SetHost()
{
    QString hostName = QInputDialog::getItem(NULL,"Connect to Host","Enter the host to connect to.",QStringList());
    if(!hostName.isEmpty())
        ConnectToHost(hostName);
}

void OpenAggregator::ConnectToHost(const QString &iHostName)
{
    if(mDatabase.isOpen())
        mDatabase.close();

    mHostName = iHostName;
    if(mHostName == "localhost")
        mDatabase = QSqlDatabase::addDatabase("QSQLITE");
    else
        mDatabase = QSqlDatabase::addDatabase("QMYSQL");
    mDatabase.setDatabaseName("Feeds");
    mDatabase.setHostName(mHostName);
    mDatabase.open();

    if(mDatabase.isOpen())
        qDebug() << "database opened";
    else
        qDebug() << mDatabase.lastError();

    if(PerformQuery("CREATE TABLE IF NOT EXISTS Feeds (url VARCHAR(500) UNIQUE PRIMARY KEY, name VARCHAR(50), folder VARCHAR(50))"))
        qDebug() << "Feeds created!";

    if(PerformQuery("CREATE TABLE IF NOT EXISTS Articles (url VARCHAR(500) UNIQUE PRIMARY KEY, feed FORIGN KEY)"))
        qDebug() << "Articles created!";

    RefreshFeedList();
}

bool OpenAggregator::PerformQuery(const QString &query)
{
    QSqlQuery qry;
    qry.prepare(query);
    if( !qry.exec() )
    {
      qDebug() << qry.lastError();
      return false;
    }
    else
        return true;
}

void OpenAggregator::RefreshFeedList()
{
    mModel.clear();

    QSqlQuery query = QSqlQuery(mDatabase);
    if(!query.exec("SELECT * From Feeds"))
        qDebug() << query.lastError();

    while(query.next())
    {
        QString name = query.value(FEEDTABLE_LOCATION_NAME).toString();
        QString folder = query.value(FEEDTABLE_LOCATION_FOLDER).toString();
        qDebug() << name << " in " << folder;

        bool folderIsInList = false;
        for(int i = 0; i < mModel.rowCount(); ++ i)
        {
            QStandardItem *folderItem = mModel.item(i);
            if(folderItem->text() ==folder)
            {
                folderIsInList = true;
                QStandardItem *feedItem = new QStandardItem;
                qDebug() << "Adding " << name;
                feedItem->setText(name);
                feedItem->setEditable(false);
                folderItem->setChild(folderItem->rowCount() ,feedItem);
            }
        }

        if(!folderIsInList)
        {
            QStandardItem *folderItem = new QStandardItem;
            folderItem->setText(folder);
            QStandardItem *feedItem = new QStandardItem;
            qDebug() << "Adding " << name << " to new Folder " << "folder";
            feedItem->setText(name);
            feedItem->setEditable(false);
            folderItem->setChild(0,feedItem);
            mModel.setItem(mModel.rowCount(),folderItem);
        }

        //download the feed
        QString urlString = query.value(FEEDTABLE_LOCATION_URL).toString();
        QUrl url(urlString);
        QNetworkRequest request(url);
        connect(&netMan,SIGNAL(finished(QNetworkReply*)),this,SLOT(DownloadFeedFinished(QNetworkReply*)));
        netMan.get(request);
    }
    mModel.setHeaderData(0,Qt::Horizontal,"Feeds");
    ui->feeds->setModel(&mModel);
}

//MANAGE FOLDERS

void OpenAggregator::FolderNameChanged(QString folder)
{
    qDebug() << folder;
}

void OpenAggregator::FeedOrFolderSelected(QModelIndex index)
{
    ui->textBrowser->clear();
    if(index.parent().row() >= 0)
    {
        QStandardItem *item = mModel.item(index.parent().row())->child(index.row());
        QString queryString = QString("SELECT url From Feeds WHERE name = '").append(item->text()).append("'");
        //is a feed
        QSqlQuery query = QSqlQuery(mDatabase);
        if(!query.exec(queryString))
            qDebug() << query.lastError();
        //feedList
        while(query.next())
        {
            qDebug() << "query url " << query.value(0).toString();
            ui->textBrowser->append(feedList[query.value(0).toString()]);
        }

    }
    else
    {
        //is a folder
        QStandardItem *item = mModel.item(index.row());
        qDebug() << item->rowCount();
        for(int i = 0; i < item->rowCount(); ++i)
            ui->textBrowser->append(item->child(i)->text());
    }
}

//MANAGE FEEDS

void OpenAggregator::Import()
{
    QString fileName = QFileDialog::getOpenFileName(NULL, "Import OPML from Google Reader", QString(), tr("XML (*.xml)"), NULL, QFileDialog::ReadOnly);
    QFile file(fileName);

    QString currentFolder = "";
    if(!fileName.isEmpty() && file.open(QIODevice::ReadOnly|QIODevice::Text))
    {

        QXmlStreamReader reader(&file);

        int depthCounter = 0;
        while(!reader.atEnd() && !reader.hasError())
        {
            QXmlStreamReader::TokenType token = reader.readNext();
            if(token == QXmlStreamReader::StartElement && reader.name() == "outline")
            {
                ++depthCounter;
                //it's a feed or a folder
                QXmlStreamAttributes attributes = reader.attributes();
                //if it's a feed
                if(attributes.hasAttribute("type") && attributes.value("type") == "rss")
                {
                    QString folderToUse = currentFolder.isEmpty() ? "main" : currentFolder;
                    QUrl url = QUrl(attributes.value("xmlUrl").toString());

                    //download the feed
                    QNetworkRequest request(url);
                    connect(&netMan,SIGNAL(finished(QNetworkReply*)),this,SLOT(DownloadFeedFinished(QNetworkReply*)));
                    netMan.get(request);

                    InsertFeed(attributes.value("title").toString(),url,folderToUse);
                }
                //if it's a folder
                else if(!attributes.hasAttribute("type"))
                {
                    currentFolder = attributes.value("title").toString();
                }

            }
            else if(token == QXmlStreamReader::EndElement && reader.name() == "outline")
            {
                --depthCounter;
                //if it's the top level we're leaving folder
                if(depthCounter==0)
                    currentFolder = "";
            }

        }
        file.close();
    }
}

void OpenAggregator::AddFeed()
{
    QString urlString = QInputDialog::getItem(NULL,"Add RSS Feed","Enter the URL of the feed.",QStringList());

    QUrl url = QUrl::fromUserInput(urlString);
    QNetworkRequest request(url);
    connect(&netMan,SIGNAL(finished(QNetworkReply*)),this,SLOT(DownloadFeedFinished(QNetworkReply*)));
    netMan.get(request);

    InsertFeed(url.toString(),url,"main");
}

void OpenAggregator::DownloadFeedFinished(QNetworkReply*reply)
{
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString feed = reply->readAll();
    if(httpStatus == 200 && feed.length())
    {
        qDebug() << "Downloaded " << reply->url();// << " to " << reply->url();
        feedList[reply->url()] = feed;
    }
    /*QXmlStreamReader xmlReader;
    xmlReader.clear();
    xmlReader.addData(reply->readAll());*/
    //UpdateFeed(reply->url(),xmlReader);
}

bool OpenAggregator::InsertFeed(const QString &name,const QUrl &url,const QString &folder)
{
    if(url.isValid())
    {
        qDebug() << "Inserting " << name << " " << url << " " << folder;
        QString query = QString("INSERT INTO Feeds (url,name,folder)");
        query.append("VALUES (");
        query.append("'").append(url.toString()).append("',");
        query.append("'").append(name).append("',");
        query.append("'").append(folder).append("'");
        query.append(")");

        qDebug() << query;
        if(PerformQuery(query))
        {
            qDebug() << "Inserted" << name << " " << url << " " << folder;
            RefreshFeedList();
            return true;
        }
    }
    return false;
}


