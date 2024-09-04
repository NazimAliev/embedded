/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

// === BOOK MODEL ---

BookModel::BookModel()
    :QSqlQueryModel()
{
}

int BookModel::rowCount(const QModelIndex &) const
{
   return QSqlQueryModel::rowCount();
}

int BookModel::columnCount(const QModelIndex &) const
{
    return QSqlQueryModel::columnCount();
}

void BookModel::setQuery(QString query)
{
    QSqlQueryModel::setQuery(query);
    DEBUG_DB;
    QSqlQueryModel::insertColumn(0);
    QSqlQueryModel::insertColumn(0);

    for(int i =0; i<rowCount(); i++)
        col0[i] = D_OFF;
}

void BookModel::setStatus(QString peer, e_disp disp)
{
    QModelIndex indexUri;

    beginResetModel();
    for(int row=0; row<rowCount(); ++row)
    {
        indexUri = index(row, COL_BOOK_URI);
        QString uri = data(indexUri).toString();
        if(uri == peer)
        {
            col0[row] = disp;
            break;
        }
    }
    endResetModel();
}

QVariant BookModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    //qDebug() << "SqlModel::data()" << role;

    if(role == Qt::DisplayRole)
    {
        int pos = col0[row];
        if(col == COL_BOOK_DISP)
            return pos;
        if(col == COL_BOOK_SDISP)
        {
            Q_ASSERT(pos>=0 && pos<statusList.size());
            return statusList.at(pos);
        }
        else
            return QSqlQueryModel::data(index, role);
    }
    if(role == Qt::FontRole)
    {
        if (row == 0 && col == 0) //change font only for cell(0,0)
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
    }
    return QVariant();
 }

