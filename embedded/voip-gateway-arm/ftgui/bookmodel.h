/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef BOOKMODEL_H
#define BOOKMODEL_H

#include <QtGui>
#include "model.h"

typedef enum
{
    COL_BOOK_DISP,
    COL_BOOK_SDISP,
    COL_BOOK_NAME,
    COL_BOOK_URI
} e_col_book;

class BookModel : public QSqlQueryModel
{
     Q_OBJECT
public:
     BookModel();
     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;
     void setQuery(QString query);
     void setStatus(QString peer, e_disp disp);
     QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
private:
     e_disp col0[NUM_BOOK];
 };

#endif // BOOKMODEL_H
