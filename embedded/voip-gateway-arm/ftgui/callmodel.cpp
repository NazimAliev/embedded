/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

// === CALLS ---

// holds information about all sessions (callList) and confs
// manage creation and deletion CallModel and CallView objects

/*
CallModel callModel;
QTableView callView;

    view1->setModel(model1);
    view1->setFixedSize(size1);
    view1->verticalHeader()->hide();
    view1->horizontalHeader()->hide();
    view1->show();
    layout->addWidget(view1);

    callBox = createPanel(&callLayout, "Calls", &callModel, &callView, QSize(200, 600));
    callView.setColumnWidth(0, 180);
    callView.hideColumn(1); // hide command
    callView.hideColumn(2); // hide URI
    callView.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&callView, SIGNAL(customContextMenuRequested(const QPoint&)), &callModel, SLOT(slotShowContextMenu(const QPoint&)));

*/

Calls::Calls(QDialog *dialog, QVBoxLayout *callLayout)
{
    Calls::dialog = dialog;
    Calls::callLayout = callLayout;
    globalMode = GLOBAL_IDLE;
    headLabel.setText("Idle");
    callLayout->addWidget(&headLabel);
}

void Calls::addCall(QString peer)
{
    CallModel* callModel = new CallModel(peer);
    QTableView* callView = new QTableView;
    callView->setModel(callModel);
    callView->setFixedSize(QSize(200, 600));
    callView->hideColumn(COL_CALL_COMMAND); // hide action
    callView->hideColumn(COL_CALL_URI); // hide URI
    callView->verticalHeader()->hide();
    callView->horizontalHeader()->hide();
    callView->setColumnWidth(0, 180);
    callView->show();
    callLayout->addWidget(callView);
    call_t* call = new(call_t);
    call->model = callModel;
    call->view = callView;
    callList.append(call);
    connect(callView, SIGNAL(clicked(QModelIndex)), dialog, SLOT(slotCallView(QModelIndex)));
}

void Calls::deleteCall(QString peer)
{
    for(int i=0; i<callList.count(); ++i)
    {
        if(callList.at(i)->model->peer == peer)
        {
            delete callList.at(i)->model;
            delete callList.at(i)->view;
            callList.removeAt(i);
            break;  // no continue!!
        }
    }
}

call_t* Calls::getCallByPeer(QString peer, QString where, bool warn)
{
    call_t* call;
    for(int i=0; i<callList.count(); ++i)
    {
        call = callList.at(i);
        if(call->model->peer == peer)
            return call;
    }
    if(warn)
        log(QString("%1: getCallByPeer can't match peer=%2").arg(where).arg(peer), LOG_WARN);
    return NULL;
}

void Calls::setGlobalMode(e_globalmode globalmode)
{
    // global option - change DA behaivor - DA click leads to REFER
    globalMode = globalmode;
    switch(globalMode)
    {
    case GLOBAL_CONF_MEMBER:
        headLabel.setText("Conference invited member");
        break;
    case GLOBAL_CONF_INITIATOR:
        headLabel.setText("Conference initiator");
        break;
    case GLOBAL_IDLE:
    default:
        headLabel.setText("Idle");
        break;
    }
}

Calls::~Calls()
{
    qDebug() << "Object Calls deleteting...";
    for(int i=0; i<callList.count(); ++i)
    {
        delete callList.at(i)->model;
        delete callList.at(i)->view;
        callList.removeAt(i);
    }
}

// === CALL MODEL ---

#define CALL_HEAD 2

CallModel::CallModel(QString peer)
    :QAbstractTableModel()
{
    //contexMenu = new QMenu();
    CallModel::peer = peer;
    status = D_OFF;
    showActions = false;
    mode = 0;
    conf = false;
    hold = false;
    div = false;
    zoomCalc();
}

int CallModel::rowCount(const QModelIndex & /*parent*/) const
{
    //qDebug() << "dataList.count()" << dataList.count();
    return rows;
}

int CallModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

void CallModel::zoomCalc()
{
    if(showActions)
        rows = actionList.count() + confList.count() + CALL_HEAD;
    else
        rows = CALL_HEAD;
}

void CallModel::addAction(e_sip sip, QString action)
{
    action_t* act = new action_t;
    act->action = action;
    act->command = sip;
    beginResetModel();
    actionList.append(act);
    zoomCalc();
    endResetModel();
}

void CallModel::deleteAction(e_sip sip)
{
    beginResetModel();
    for(int i=0; i<actionList.count(); ++i)
    {
        if(actionList.at(i)->command == sip)
        {
            actionList.removeAt(i);
        }
    }
    zoomCalc();
    endResetModel();
}

void CallModel::addMember(QString peer)
{
    confList.append(peer);
    zoomCalc();
    log(QString("parseNotify: conf member [%1] added to call").arg(peer), LOG_OTHER);
}

void CallModel::deleteMember(QString peer)
{
    if(confList.removeAll(peer) == 0)
        log(QString("parseNotify: Can't delete peer:%1").arg(peer), LOG_WARN);
    else
    {
        zoomCalc();
        log(QString("parseNotify: conf member [%1] removed from call").arg(peer), LOG_OTHER);
    }
}

void CallModel::setStatus(e_disp disp)
{
    beginResetModel();
    status = disp;
    endResetModel();
}

void CallModel::parseNotify(QString body)
{
    // converts notify body to string list of peer to indicate conf members in context call menu
    QString str;
    QString header;
    QString action;
    QString peer;

    QStringList list = body.split('/', QString::SkipEmptyParts);
    qDebug() << "parseNotify: strings in body:" << list.count();
    beginResetModel();
    for(int i=0; i<list.count(); ++i)
    {
        str = list.at(i);
        header = str.section(',', 0, 0);
        action = str.section(',', 1, 1);
        peer = str.section(',', 2, 2);
            continue;
        if(header != "FTCONF")
        {
            log(QString("parseNotify: Undefined header:%1").arg(header), LOG_WARN);
            break;
        }
        if(action == "+")
        {
            addMember(peer);
        }
        else if(action == "-" && peer != Uri)
        {
            deleteMember(peer);
        }
        else
        {
            log(QString("parseNotify: Undefined action:%1").arg(action), LOG_WARN);
        }
    }
    endResetModel();
}

QVariant CallModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    int actionCount = actionList.count();
    int confCount = confList.count();
    Q_ASSERT(row>=0 && row<actionCount+confCount+CALL_HEAD);
    QVariant res = QVariant();

    if(role == Qt::FontRole && row == 0)
    {
        QFont boldFont;
        boldFont.setBold(true);
        res = boldFont;
    }

    if(role == Qt::DisplayRole && row < CALL_HEAD)
    {
        // display only head rows - URI and status
        QString tmp = peer;
        switch(col)
        {
        case COL_CALL_ACTION:
            if(row == 0)
            {
                res = tmp.remove(QRegExp("@.*"));
            }
            else
            {
                Q_ASSERT(status>=0 && status<statusList.count());
                res = statusList.at(status);
            }
            break;
        case COL_CALL_COMMAND:
            res = E_NULL;
            break;
        case COL_CALL_URI:
        default:
            res = peer;
            break;
        }
    }
    row -= CALL_HEAD;
    if(role == Qt::DisplayRole && row >= 0 && row < actionCount)
    {
        // display actions
        switch(col)
        {
        case COL_CALL_ACTION:
            res = actionList.at(row)->action;
            break;
        case COL_CALL_COMMAND:
            res = actionList.at(row)->command;
            break;
        case COL_CALL_URI:
        default:
            res = peer;
            break;
        }
    }
    if(role == Qt::DisplayRole && row >= actionCount && row < actionCount + confCount)
    {
        row-=actionCount;
        switch(col)
        {
        case COL_CALL_ACTION:
            res = confList.at(row);
            break;
        case COL_CALL_COMMAND:
            res = E_NULL;
            break;
        case COL_CALL_URI:
        default:
            res = E_NULL;
            break;
        }
    }
    return res;
 }

bool CallModel::setData(const QModelIndex, const QVariant, int)
{
    // index doesn't matter. click in head changes zoom

    beginResetModel();
    if(showActions)
        showActions = false;
    else
        showActions = true;
    zoomCalc();
    endResetModel();
    return true;
}

void CallModel::slotShowContextMenu(const QPoint&)
{
    // for most widgets
    // QPoint globalPos = myWidget->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    contexMenu->addAction("Menu Item 1");
    contexMenu->addAction("Menu Item 2");
    // ...

    //QAction* selectedItem = contexMenu.exec(globalPos);
    QAction* selectedItem = contexMenu->exec(QCursor::pos());
    if (selectedItem)
    {
        // something was chosen, do stuff
        qDebug() << selectedItem->text();
    }
    else
    {
        // nothing was chosen
    }

}

CallModel::~CallModel()
{
    // qDebug() << "CallModel deleting...";
    for(int i=0; i<actionList.count(); ++i)
    {
        actionList.removeAt(i);
    }
    confList.clear();
}
