/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef CALLMODEL_H
#define CALLMODEL_H

#include <QtGui>
#include "model.h"

class CallModel;

typedef struct
{
    CallModel* model;
    QTableView* view;
} call_t;

// global roles at the time
typedef enum
{
    GLOBAL_IDLE,
    GLOBAL_CONF_MEMBER,
    GLOBAL_CONF_INITIATOR
} e_globalmode;

class Calls : public QObject
{
    Q_OBJECT
public:
    Calls(QDialog* dialog, QVBoxLayout* callLayout);
private:
    QDialog* dialog;
    QVBoxLayout* callLayout;
    QLabel headLabel;
    QList<call_t*> callList;
    e_globalmode globalMode;
public:
    call_t* getCallByPeer(QString peer, QString where, bool warn=true);
    void addCall(QString peer);
    void setGlobalMode(e_globalmode globalmode);
    e_globalmode getGlobalMode(){return globalMode;}
    void deleteCall(QString peer);

    ~Calls();
};

typedef struct
{
    QString action;
    e_sip command;
} action_t;

typedef enum
{
    COL_CALL_ACTION,
    COL_CALL_COMMAND,
    COL_CALL_URI
} e_col_call;

class CallModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CallModel(QString peer);
    QString peer;
    e_disp status;
    bool showActions;
    int mode;
    bool conf;
    bool hold;
    bool div;
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void zoomCalc();
    void addAction(e_sip sip, QString action);
    void deleteAction(e_sip sip);
    void addMember(QString peer);
    void deleteMember(QString peer);
    int membersCount() {return confList.count();}
    void setStatus(e_disp disp);
    void parseNotify(QString body);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData (const QModelIndex, const QVariant, int  = Qt::EditRole);
    ~CallModel();
private:
    int rows;
    QList<action_t*> actionList;
    QStringList confList;
    QMenu* contexMenu;
public slots:
    void slotShowContextMenu(const QPoint&);
};

#endif // CALLMODEL_H
