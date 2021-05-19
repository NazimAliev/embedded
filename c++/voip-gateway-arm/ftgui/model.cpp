/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

QString Book = "SELECT name_tech, uri FROM book WHERE type=0";

// radio panel view: key, freq in MHz, human name of freq
QString Freq = "SELECT DISTINCT freq.key, freq.freq, freq.name, freq.rmode "
        "FROM freq";

// all radios for given freq
QString Radio = "SELECT radio.name, radio.isOn, radio.id, radio.ref_freq, radio.isRx, radio.isTx, radio.uri, isCoupled, sn_method "
        "FROM radio "
        "WHERE radio.ref_freq=%1 AND "
        "radio.isOn>=0 "
        "ORDER BY radio.name";

// only Rx capability radios for given freq
QString Radio_rx = "SELECT radio.name, radio.isOn, radio.id, radio.ref_freq, radio.isRx, radio.isTx "
        "FROM radio "
        "WHERE radio.ref_freq=%1 AND "
        "radio.isOn>=1 AND radio.isRx=1 "
        "ORDER BY radio.name";

// only Tx capability radios for given freq
QString Radio_tx = "SELECT radio.name, radio.isOn, radio.id, radio.ref_freq, radio.isRx, radio.isTx "
        "FROM radio "
        "WHERE radio.ref_freq=%1 AND "
        "radio.isOn>=1 AND radio.isTx=1 "
        "ORDER BY radio.name";

// bss for selecting instead of Rx
QString Bss = "SELECT DISTINCT bss.name, bss.isOn, link_bss.ref_bss "
        "FROM bss, link_bss "
        "WHERE link_bss.ref_bss = bss.key AND link_bss.ref_radio_freq=%1";

// cli for selecting instead of Rx
QString Cli = "SELECT DISTINCT cli.name, cli.isOn, link_cli.ref_cli "
        "FROM cli, link_cli "
        "WHERE link_cli.ref_cli = cli.key AND link_cli.ref_radio_freq=%1";

// groups name to show in Label (left side of group)
QString Label_bss = "SELECT DISTINCT bss.name, link_bss.ref_bss "
        "FROM bss, link_bss "
        "WHERE link_bss.ref_bss = bss.key AND link_bss.ref_radio_freq=%1";

QString Label_cli = "SELECT DISTINCT cli.name, link_cli.ref_cli "
        "FROM cli, link_cli "
        "WHERE link_cli.ref_cli = cli.key AND link_cli.ref_radio_freq=%1";

// radios show depended of selected group
QString Label_bss_radio = "SELECT radio.name, radio.id, radio.ref_freq "
        "FROM radio, bss, link_bss "
        "WHERE radio.id = link_bss.ref_radio_id AND "
        "radio.ref_freq = link_bss.ref_radio_freq AND "
        "link_bss.ref_bss = bss.key AND bss.isOn >= 0 AND bss.key = %1";

QString Label_cli_radio = "SELECT radio.name, radio.id, radio.ref_freq "
        "FROM radio, cli, link_cli "
        "WHERE radio.id = link_cli.ref_radio_id AND "
        "radio.ref_freq = link_cli.ref_radio_freq AND "
        "link_cli.ref_cli = cli.key AND cli.isOn >= 0 AND cli.key = %1";

QString Update_radio = "UPDATE radio "
        "SET isOn=? "
        "WHERE radio.id=? AND "
        "radio.ref_freq=?";

// ToDo: set isRx
QString Update_rx = "UPDATE radio "
        "SET isOn=? "
        "WHERE radio.isOn>=1 AND "
        "radio.id=? AND "
        "radio.ref_freq=?";

// ToDo: set isTx
QString Update_tx =  "UPDATE radio "
        "SET isOn=? "
        "WHERE radio.isOn>=1 AND "
        "radio.id=? AND "
        "radio.ref_freq=?";

QString Update_bss = "UPDATE bss "
        "SET isOn=? "
        "WHERE key=?";

QString Insert_bss = "INSERT INTO link_bss "
        "VALUES (%1, ?, ?)";

QString Delete_bss = "DELETE FROM link_bss "
        "WHERE link_bss.ref_radio_id=? AND link_bss.ref_radio_freq=? AND "
        "link_bss.ref_bss = %1";

QString Update_cli = "UPDATE cli "
        "SET isOn=? "
        "WHERE key=?";

QString Insert_cli = "INSERT INTO link_cli "
        "VALUES (%1, ?, ?)";

QString Delete_cli = "DELETE FROM link_cli "
        "WHERE link_cli.ref_radio_id=? AND link_cli.ref_radio_freq=? AND "
        "link_cli.ref_cli = %1";

QString Uri;

// attention - getX() functions return internal model variables that don't retrieve data() function.
// getX() - for external use variables not shown by data() function
// data() - only for using in model view
// if is necessary use variables than already play in data() function - use data() function, no create new getX() function


// === Simple MENU MODEL ---

SimpleModel::SimpleModel()
    :QSqlQueryModel()
{
    rows = -1;
    cols = -1;

    for(int i=0; i<NUM_BSS; i++)
    {
        bss[i] = "";
        isOn[i] = -1;
        key[i] = -1;
    }
}

int SimpleModel::rowCount(const QModelIndex &) const
{
   return rows;
}

int SimpleModel::columnCount(const QModelIndex &) const
{
    return cols;
}

void SimpleModel::setQuery(QString query)
{
    beginResetModel();

    QSqlQueryModel::setQuery(query);
    rows = QSqlQueryModel::rowCount();
    cols = QSqlQueryModel::columnCount();

    Q_ASSERT(rows < NUM_BSS);

    QModelIndex index;

    for(int i=0; i<rows; i++)
    {
        // retrieve radio name
        index = QSqlQueryModel::index(i, 0);
        bss[i] = QSqlQueryModel::data(index).toString();
        index = QSqlQueryModel::index(i, 1);
        if(QSqlQueryModel::data(index).toInt() == 1)
            isOn[i] = true;
        else
            isOn[i] = false;
        index = QSqlQueryModel::index(i, 2);
        key[i] = QSqlQueryModel::data(index).toInt();
    }

    cols = rows;
    rows = 1;

    endResetModel();
}

void SimpleModel::setUpdate(QString query)
{
    Q_ASSERT(queryUpdate.prepare(query));
}

QVariant SimpleModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    bool selected;

    int col = index.column();

    switch(role)
    {
    case Qt::DisplayRole:
        value = bss[col] + " [" + QString("%1").arg(key[col]) + "]";
        return value;
        break;
    case Qt::FontRole:
        selected = isOn[col];
        if (selected)
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
        break;
    default:
        break;
    }
    return QVariant();
}

bool SimpleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int col = index.column();

    queryUpdate.bindValue(0, value.toInt());  // isOn
    queryUpdate.bindValue(1, key[col]);  // bss.key
    Q_ASSERT(queryUpdate.exec());
    isOn[col] = value.toInt();

    Q_EMIT(dataChanged(index, index));

    return QSqlQueryModel::setData(index, value, role);
}

// === Label MENU MODEL ---

LabelModel::LabelModel()
    :QSqlQueryModel()
{
    rows = -1;
    cols = -1;

    for(int i=0; i<NUM_BSS; i++)
    {
        label[i] = "";
        key[i] = -1;
    }
}

int LabelModel::rowCount(const QModelIndex &) const
{
   return rows;
}

int LabelModel::columnCount(const QModelIndex &) const
{
    return cols;
}

void LabelModel::setQuery(QString query)
{
    beginResetModel();

    QSqlQueryModel::setQuery(query);
    rows = QSqlQueryModel::rowCount();
    cols = QSqlQueryModel::columnCount();

    Q_ASSERT(rows < NUM_BSS);

    QModelIndex index;

    for(int i=0; i<rows; i++)
    {
        // retrieve radio name
        index = QSqlQueryModel::index(i, 0);
        label[i] = QSqlQueryModel::data(index).toString();
        index = QSqlQueryModel::index(i, 1);
        key[i] = QSqlQueryModel::data(index).toInt();
    }

    label[rows] = "+";
    key[rows] = -1;
    rows ++;    // for placing "+" sign
    cols = 1;

    endResetModel();
}

void LabelModel::setUpdate(QString squery)
{
    Q_ASSERT(queryUpdate.prepare(squery));
}

QVariant LabelModel::data(const QModelIndex &index, int role) const
{
    QVariant value;

    int row = index.row();

    switch(role)
    {
    case Qt::DisplayRole:
        value = label[row] + " [" + QString("%1").arg(key[row]) + "]";
        return value;
        break;
    default:
        break;
    }
    return QVariant();
}

bool LabelModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();

    queryUpdate.bindValue(0, value.toInt());  // data
    queryUpdate.bindValue(1, key[row]);  // bss.key
    Q_ASSERT(queryUpdate.exec());

    Q_EMIT(dataChanged(index, index));

    return QSqlQueryModel::setData(index, value, role);
}

// === SubMENU MODEL ---

SubmenuModel::SubmenuModel()
    :QAbstractTableModel()
{
    rows = 1;
    mode = 0;
}

void SubmenuModel::setMode(int mode)
{
    beginResetModel();
    QStringList tmp;
    tmp << "Ok" << "X";

    switch(mode)
    {
    case 0:
        tmp << "" << "MG" << "CG" << "" << "Off" << "Rx" << "RxTx";
        break;
    case 1:
        break;
    case 2:
        tmp << "" << "Del";
        break;
    default:
        break;
    }
    stringList = tmp;
    cols = stringList.size();
    endResetModel();
}

int SubmenuModel::rowCount(const QModelIndex & /*parent*/) const
{
   return rows;
}

int SubmenuModel::columnCount(const QModelIndex & /*parent*/) const
{
    return cols;
}

QVariant SubmenuModel::data(const QModelIndex &index, int role) const
{
    //int row = index.row();
    int col = index.column();

    if(role == Qt::DisplayRole)
    {
        return stringList.at(col);
    }
    return QVariant();
 }

