#ifndef FOLDERVIEWMODEL_H
#define FOLDERVIEWMODEL_H
#include <QAbstractProxyModel>

class FolderViewModel : public QAbstractProxyModel
{
    Q_OBJECT
    public:
    FolderViewModel(QObject* parent = 0) : QAbstractProxyModel(parent) { }

    QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const{return sourceIndex;}

   QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const{return proxyIndex;}

   int rowCount(const QModelIndex& parent) const{return sourceModel()->rowCount(parent);}

   int columnCount(const QModelIndex& parent) const{return sourceModel()->columnCount(parent);}

   QModelIndex index(int row, int col, const QModelIndex& parent) const{return sourceModel()->index(row, col, parent);}
   QModelIndex parent(const QModelIndex& index) const{return parent(index);}
};
#endif // FOLDERVIEWMODEL_H
