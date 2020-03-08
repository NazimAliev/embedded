/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

// === RADIO MODEL ---

RadioModel::RadioModel()
    :QSqlQueryModel()
{
    rows = -1;
    cols = -1;
    limiter = 6;
    m_isOnLevel = 0;
    for(int i=0; i<NUM_RADIO; i++)
    {
        name[i] = "";
        uri[i] = "";
        isOn[i] = -1;
        isRx[i] = -1;
        isTx[i] = -1;
        id[i] = -1;
        ref_freq[i] = -1;
    }
}

int RadioModel::rowCount(const QModelIndex &) const
{
   return rows;
}

int RadioModel::columnCount(const QModelIndex &) const
{
    return cols;
}

void RadioModel::setQuery(QString query)
{
    beginResetModel();

    QSqlQueryModel::setQuery(query);
    DEBUG_DB;
    o_rows = QSqlQueryModel::rowCount();
    Q_ASSERT(o_rows < NUM_RADIO);

    // IMPORTANT - sql rows/cols should be set before we will use index
    rows = QSqlQueryModel::rowCount();
    cols = QSqlQueryModel::columnCount();   //don't remove this line dispite cols doesn't used below!!!

    QModelIndex sqlIndex;

    for(int i=0; i<o_rows; i++)
    {
        // retrieve radio name
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_NAME);
        name[i] = QSqlQueryModel::data(sqlIndex).toString();

        // retrieve isOn
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_ISON);
        isOn[i] = QSqlQueryModel::data(sqlIndex).toInt();

        // retrieve radio keys
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_ID);
        id[i] = QSqlQueryModel::data(sqlIndex).toInt();
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_REF_FREQ);
        ref_freq[i] = QSqlQueryModel::data(sqlIndex).toInt();

        QSqlQuery queryFreq;
        queryFreq.prepare("SELECT freq FROM freq WHERE key=?");
        queryFreq.addBindValue(ref_freq[i]);
        queryFreq.exec();
        if(queryFreq.first())
            freq[i] = queryFreq.value(0).toString();

        // retrieve Rx/Tx mode
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_ISRX);
        isRx[i] = QSqlQueryModel::data(sqlIndex).toInt();
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_ISTX);
        isTx[i] = QSqlQueryModel::data(sqlIndex).toInt();

        // retrieve URI
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_URI);
        uri[i] = QSqlQueryModel::data(sqlIndex).toString();

        // retrieve isCoupled
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_ISCOUPLED);
        isCoupled[i] = QSqlQueryModel::data(sqlIndex).toInt();

        // retrieve signal/noise method
        sqlIndex = QSqlQueryModel::index(i, COL_RADIO_SNMETHOD);
        snMethod[i] = QSqlQueryModel::data(sqlIndex).toString();
    }

    // IMPORTANT - we can redefine rows/cols only after using index
    if(rows > limiter)
        cols = limiter;
    else
        cols = o_rows;

    rows = o_rows/limiter;
    if(o_rows % limiter > 0)
        rows++;

    endResetModel();

}

QVariant RadioModel::getVar(int idx, e_col_radio var)
{
    Q_ASSERT(idx>=0 && idx<=o_rows);
    switch(var)
    {
    case COL_RADIO_NAME:
        return name[idx];
    case COL_RADIO_ISON:
        return isOn[idx];
    case COL_RADIO_ID:
        return id[idx];
    case COL_RADIO_REF_FREQ:
        return ref_freq[idx];
    case COL_RADIO_ISRX:
        return isRx[idx];
    case COL_RADIO_ISTX:
        return isTx[idx];
    case COL_RADIO_URI:
        return uri[idx];
    case COL_RADIO_ISCOUPLED:
        return isCoupled[idx];
    case COL_RADIO_SNMETHOD:
        return snMethod[idx];
    case COL_RADIO_FREQ:
        return freq[idx];
    default:
        return false;
    }

}

void RadioModel::setUpdate(QString query)
{
    Q_ASSERT(queryUpdate.prepare(query));
}

QVariant RadioModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    bool selected;

    int row = index.row();
    int col = index.column();
    int idx = limiter*row + col;

    QString prefix = "-- ";

    switch(role)
    {
    case Qt::DisplayRole:
        if(isRx[idx] == 1)
            prefix = "Rx ";
        if(isTx[idx] == 1)
            prefix = "Tx ";
        if(isRx[idx] == 1 && isTx[idx] == 1)
            prefix = "RxTx ";
        value = prefix += name[idx];
        //qDebug() << "RadioModel::data" << value << index;
        return value;
        break;

    case Qt::FontRole:
        selected = (isOn[idx] > m_isOnLevel);
        if (selected)
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
        break;
    }
    return QVariant();
 }

bool RadioModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // value=1 if index() item was selected before Ok cliked. otherwise value=0
    int row = index.row();
    int col = index.column();
    int idx = limiter*row + col;
    int level;

    if(value.toInt() == 1)
        level = m_isOnLevel+1;
    else
        level = m_isOnLevel;

    //qDebug() << "RadioModel::setData" << index << value;
    queryUpdate.bindValue(0, level);                // isOn
    queryUpdate.bindValue(1, id[idx]);            // link_book.ref_radio
    queryUpdate.bindValue(2, ref_freq[idx]);    // link_book.ref_radio
    Q_ASSERT(queryUpdate.exec());
    isOn[idx] = level;
    Q_EMIT(dataChanged(index, index));

    return QSqlQueryModel::setData(index, value, role);
}

// === BSS and CLI GROUP MODEL ---

RadioChildModel::RadioChildModel()
    :RadioModel()
{
}

void RadioChildModel::setQuery(QString query, QString queryBase)
{
    int child_radio_id;
    int child_radio_ref_freq;

    beginResetModel();

    // base SELECT independed from bss, cli - all available radios
    RadioModel::setQuery(queryBase);

    QSqlQuery childQuery;
    Q_ASSERT(childQuery.prepare(query));
    for(int i=0; i<o_rows; i++)
    {
        isOn[i] = 0;
        Q_ASSERT(childQuery.exec());
        while(childQuery.next())
        {
            child_radio_id = childQuery.value(1).toInt();
            child_radio_ref_freq = childQuery.value(2).toInt();
            if(child_radio_id != id[i])
                continue;
            if(child_radio_ref_freq != ref_freq[i])
                continue;
            isOn[i] = 1;
            break;
        }
    }

    endResetModel();

}

void RadioChildModel::setUpdate(QString queryInsert, QString queryDelete)
{
    Q_ASSERT(queryIns.prepare(queryInsert));
    Q_ASSERT(queryDel.prepare(queryDelete));
}

bool RadioChildModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // value=1 if index() item was selected before Ok cliked. otherwise value=0

    int row = index.row();
    int col = index.column();
    int idx = limiter*row + col;

    if(value == isOn[idx])
        // no changes - no job
        return QSqlQueryModel::setData(index, value, role);

    //qDebug() << "RadioChildModel::setData changed value:" << value << "isOn:" << isOn[idx] << index;
    // INSERT of DELETE?
    if(value == 1)
    {
        // value=1, isOn=0. New radio was selected - INSERT
        pQueryUpdate = &queryIns;
        isOn[idx] = 1;
    }
    else
    {
        // value=0, isOn=1. Current radio was deselected - DELETE
        pQueryUpdate = &queryDel;
        isOn[idx] = 0;
    }

    pQueryUpdate->bindValue(0, id[idx]);
    pQueryUpdate->bindValue(1, ref_freq[idx]);
    Q_ASSERT(pQueryUpdate->exec());
    Q_EMIT(dataChanged(index, index));

    return QSqlQueryModel::setData(index, value, role);
}

