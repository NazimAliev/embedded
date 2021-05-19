/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

// === delegate for painting freq model ---

FreqViewDelegate::FreqViewDelegate()
    :QItemDelegate()
{
    radioFrame = new RadioFrame;
    // qDebug() << "Create RadioFrame";
}

void FreqViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qDebug() << "paint" << index;

    painter->save();
    if (option.state & QStyle::State_Selected)
        radioFrame->setSelect(true);
    else
        radioFrame->setSelect(false);
    QString list = index.data().toString();
    radioFrame->setFields(list);
    radioFrame->resize(option.rect.size());
    // qDebug() << "resize" << option.rect.size();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->translate(option.rect.topLeft());
    radioFrame->render(painter);
    painter->restore();
}

QSize FreqViewDelegate::sizeHint(const QStyleOptionViewItem, const QModelIndex) const
{
    int w = RCELL_W;
    int h = RCELL_H;
    qDebug() << "sizeHint";
    return QSize(w, h);
}

FreqViewDelegate::~FreqViewDelegate()
{
    delete radioFrame;
}

// === SQL FREQ MODEL ---

FreqModel::FreqModel(QObject *parent)
    :QSqlQueryModel(parent)
{
    rows = -1;
    cols = -1;
    for(int i=0; i<NUM_FREQ; i++)
    {
        mode_level[i] = F_OFF;
        mode[i] = F_OFF;
        config[i] = false;
        led[i] = Qt::red;
    }
}

int FreqModel::rowCount(const QModelIndex &) const
{
   return rows;
}

int FreqModel::columnCount(const QModelIndex &) const
{
    return cols;
}

void FreqModel::setQuery(QString query)
{
    beginResetModel();

    QSqlQueryModel::setQuery(query);
    DEBUG_DB;
    o_rows = QSqlQueryModel::rowCount();
    Q_ASSERT(o_rows < NUM_FREQ);

    // IMPORTANT - sql rows/cols should be set before we will use index
    rows = QSqlQueryModel::rowCount();
    cols = QSqlQueryModel::columnCount();

    QModelIndex sqlIndex;

    for(int i=0; i<o_rows; i++)
    {
        // retrieve key
        sqlIndex = QSqlQueryModel::index(i, COL_FREQ_KEY);
        key[i] = QSqlQueryModel::data(sqlIndex).toInt();

        // retrieve isOn
        sqlIndex = QSqlQueryModel::index(i, COL_FREQ_FREQ);
        freq[i] = QSqlQueryModel::data(sqlIndex).toString();

        // retrieve radio keys
        sqlIndex = QSqlQueryModel::index(i, COL_FREQ_NAME);
        name[i] = QSqlQueryModel::data(sqlIndex).toString();

        // ToDo - initialization depended on permitted Rx/Tx
        sqlIndex = QSqlQueryModel::index(i, COL_FREQ_MODE);
        mode_level[i] = static_cast<e_freq>(QSqlQueryModel::data(sqlIndex).toInt());
        mode[i] = mode_level[i];
    }

    rows = 1;   // don't replace it to constructor - should be here
    cols = o_rows;

    endResetModel();
}

e_freq FreqModel::getMode(int freq_key)
{
    int freq = freq_key - MIN_FREQ;
    Q_ASSERT(freq >=0 && freq <= NUM_FREQ);
    return mode[freq];
}

void FreqModel::setMode(int freq_key, e_freq sMode)
{
    int freq = freq_key - MIN_FREQ;
    Q_ASSERT(freq >=0 && freq <= NUM_FREQ);
    Q_ASSERT(sMode >= F_OFF && sMode <=F_RXTX);
    beginResetModel();
    mode_level[freq] = sMode;
    mode[freq] = sMode;
    QSqlQuery queryUpdate;
    QString query = "UPDATE freq SET rmode=? WHERE key=?";
    Q_ASSERT(queryUpdate.prepare(query));
    queryUpdate.bindValue(0, sMode);
    queryUpdate.bindValue(1, freq_key);
    Q_ASSERT(queryUpdate.exec());
    endResetModel();
}

int FreqModel::getFreq(int col)
{
    Q_ASSERT(col >=0 && col <= NUM_FREQ);
    return key[col];
}

QVariant FreqModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
    QVariant value;

    switch(role)
    {
    case Qt::DisplayRole:
        value = QString("%1;%2;%3;%4;%5").arg(config[col]).arg(freq[col]).arg(name[col]).arg(mode[col]).arg(led[col]);
        return value;
        break;
    case Qt::SizeHintRole:
        return QSize(RCELL_W, RCELL_H);
        break;
    }
    return QVariant();
 }

bool FreqModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // mode_level value:
    // 0 - freq disabled (grayed), can be turned on only from Radio page
    // 1 - Rx only freq. No possibility to switch as to OFF as to Tx
    // 2 - RxTx freq. Switch between Rx and Tx every click
    int col = index.column();
    beginResetModel();
    switch(role)
    {
    case ROLE_FREQ_MODE:
        // value doesn't matter - only toggle Rx/Tx
        config[col] = false;
        if(mode_level[col] < F_RXTX)
            // Rx-only or disabled - no switch
            break;

        // switch between Rx and RxTx
        if(mode[col] == F_RX)
            mode[col] = F_RXTX;
        else
            mode[col] = F_RX;
        break;
    case ROLE_FREQ_CONFIG:
        config[col] = value.toInt();
        break;
    case ROLE_FREQ_LED:
        led[col] = static_cast<Qt::GlobalColor>(value.toInt());
        break;
    }
    endResetModel();
    // qDebug() << index << value << mode[col] << mode_level[col];

    // Q_EMIT(dataChanged(index, index));

    return QSqlQueryModel::setData(index, value, role);
}

FreqModel::~FreqModel()
{

}

