/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef MODEL_H
#define MODEL_H

#define DEBUG_DB if (lastError().isValid()) qDebug() << lastError();

#include <QtGui>
#include <QtSql>
#include <QUdpSocket>

#include "intersip.h"

extern QString Book;
extern QString Freq;
extern QString Radio;
extern QString Radio_rx;
extern QString Radio_tx;
extern QString Bss;
extern QString Cli;
extern QString Label_bss;
extern QString Label_cli;
extern QString Label_bss_radio;
extern QString Label_cli_radio;
extern QString Update_radio;
extern QString Update_rx;
extern QString Update_tx;
extern QString Update_bss;
extern QString Insert_bss;
extern QString Delete_bss;
extern QString Update_cli;
extern QString Insert_cli;
extern QString Delete_cli;

extern QString Uri;

extern QStringList statusList;

typedef enum
{
    R_DISABLED,
    R_ENABLED,
    R_SELECTED
} e_radio;

typedef enum
{
    F_OFF,
    F_RX,
    F_RXTX
} e_freq;

class LabelModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    LabelModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void setQuery(QString query);
    void setUpdate(QString squery);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
private:
    int rows;
    int cols;
    QString label[NUM_BSS];
    int key[NUM_BSS];
    QSqlQuery queryUpdate;
};

class SubmenuModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    void setMode(int mode);
    SubmenuModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
private:
    int rows;
    int cols;
    int mode;
    QStringList stringList;
};



#endif // MODEL_H
