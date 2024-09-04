/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef FREQMODEL_H
#define FREQMODEL_H

#include <QtGui>
#include <QSqlQueryModel>
#include "model.h"
#include "widget.h"

class FreqViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    FreqViewDelegate();
    ~FreqViewDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem, const QModelIndex) const;
private:
    RadioFrame* radioFrame;
};

typedef enum
{
    COL_FREQ_KEY,
    COL_FREQ_FREQ,
    COL_FREQ_NAME,
    COL_FREQ_MODE
} e_col_freq;

typedef enum
{
    ROLE_FREQ_MODE,
    ROLE_FREQ_CONFIG,
    ROLE_FREQ_LED
} e_role_freq;

class FreqModel : public QSqlQueryModel
{
     Q_OBJECT
public:
     FreqModel(QObject *parent = 0);
     ~FreqModel();
     int rowCount(const QModelIndex &parent = QModelIndex()) const ;
     int columnCount(const QModelIndex & = QModelIndex()) const;
     void setQuery(QString query);
     e_freq getMode(int freq_key);
     void setMode(int freq_key, e_freq sMode);
     int getFreq(int col);
     QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
     bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
private:
     int key[NUM_FREQ];
     QString freq[NUM_FREQ];
     QString name[NUM_FREQ];
     e_freq mode_level[NUM_FREQ];
     e_freq mode[NUM_FREQ];
     bool config[NUM_FREQ];
     Qt::GlobalColor led[NUM_FREQ];
     int rows;
     int cols;
     int o_rows;
 };

#endif // FREQMODEL_H
