/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include <QtGui>
#include "model.h"

typedef enum
{
    COL_RADIO_NAME,
    COL_RADIO_ISON,
    COL_RADIO_ID,
    COL_RADIO_REF_FREQ,
    COL_RADIO_ISRX,
    COL_RADIO_ISTX,
    COL_RADIO_URI,
    COL_RADIO_ISCOUPLED,
    COL_RADIO_SNMETHOD,
    COL_RADIO_FREQ
} e_col_radio;

class RadioModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    RadioModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void setLevel(int isOnLevel) {m_isOnLevel = isOnLevel;}
    void setQuery(QString query);
    QVariant getVar(int idx, e_col_radio var);
    int getNumVars() {return o_rows;}
    void setUpdate(QString query);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
private:
    int rows;
    int cols;
    QSqlQuery queryUpdate;
    int m_isOnLevel;
    QString name[NUM_RADIO];
protected:
    int o_rows;
    int limiter;
    int id[NUM_RADIO];
    int ref_freq[NUM_RADIO];
    QString freq[NUM_RADIO];
    int isOn[NUM_RADIO];
    int isRx[NUM_RADIO];
    int isTx[NUM_RADIO];
    QString uri[NUM_RADIO];
    int isCoupled[NUM_RADIO];
    QString snMethod[NUM_RADIO];
};

class RadioChildModel : public RadioModel
{
    Q_OBJECT
public:
    RadioChildModel();
    void setQuery(QString query, QString queryBase);
    void setUpdate(QString queryInsert, QString queryDelete);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
private:
    QSqlQuery queryIns;
    QSqlQuery queryDel;
    QSqlQuery* pQueryUpdate;
};

class SimpleModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    SimpleModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void setQuery(QString query);
    void setUpdate(QString query);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
private:
    int rows;
    int cols;
    QString bss[NUM_BSS];
    int key[NUM_BSS];
    int isOn[NUM_BSS];
    QSqlQuery queryUpdate;
};

#endif // RADIOMODEL_H
