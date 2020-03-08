/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setFixedSize(1000, 650);
    current_menu = G_ADDR;
    current_freq = MIN_FREQ;
    current_bss = MIN_BSS;
    current_cli = MIN_CLI;
    menuWidget = new QWidget;
    calls = new Calls(this, &callLayout);
}

bool Dialog::isIdle()
{
    bool cond = TRUE;
    QModelIndex indexDisp;

    for (int i=0; i < bookModel.rowCount(); ++i)
    {
        indexDisp = bookModel.index(i, COL_BOOK_SDISP);
        e_disp disp = (e_disp)bookModel.data(indexDisp).toInt();
        if (disp == D_OFF)
        {
            continue;
        }
        else
        {
            cond = FALSE;
            break;
        }
    }
    return cond;
}

void Dialog::slotCallView(QModelIndex index)
{
    const QAbstractItemModel* itemModel = index.model();
    QModelIndex workIndex;
    call_t* call;
    int row = index.row();

    //QModelIndex workIndex = itemModel->index(row, COL_CALL_ACTION); // for debug onlye
    workIndex = itemModel->index(row, COL_CALL_COMMAND);
    e_sip command = static_cast <e_sip>(workIndex.data().toInt());
    workIndex = itemModel->index(row, COL_CALL_URI);
    QString peer = workIndex.data().toString();

    switch(command)
    {
    case E_NULL:
        // click parent row - this only for zoomin
        call = calls->getCallByPeer(peer, "slotCallView");
        Q_ASSERT(call);
        workIndex = itemModel->index(row, COL_CALL_ACTION);
        call->model->setData(workIndex, 0);
        break;
    case EH_CONF_START:
        // connect to focus
        // can't pass peer and replaces to eventChain directly
        chain_refer = peer;
        chain_replaces = QString("%1/%2").arg(Uri).arg(chain_refer);
        eventChain(conf_uri, EH_DA, E_NULL, "", "", "");
        break;
    default:
        eventChain(peer, command, E_NULL, "", "", "");
    }
}

void Dialog::slotFreqView(QModelIndex index)
{
    // user clicked on freq panel - action depended on current_menu mode
    int col = index.column();
    //current_freq = freqModel.index(0, col).data().toInt();
    current_freq = freqModel.getFreq(col);

    if(current_menu == G_ADDR)
    {
        // addr book panel is visible
        // switch between Rx and Tx if allowed by mode_level inside model

        // ToDo: set mode should be only after reply from peer
        freqModel.setData(index, 0, ROLE_FREQ_MODE);
        // process event
        hmiRadioEvent(current_freq);
    }
    else
    {
        freqModel.setData(index, true, ROLE_FREQ_CONFIG);
    }
    refreshGroup();
}

void Dialog::slotBookView(QModelIndex index)
{
    // user clicked on addr book - make a call. index.row() is number of record in addr book
    //qDebug() << "slotBookView:index" << index;
    int row = index.row();
    QModelIndex indexUri = bookModel.index(row, COL_BOOK_URI);
    QString peer = bookModel.data(indexUri).toString();

    // in conf mode DA buttons become conf invite buttons
    if(calls->getGlobalMode() == GLOBAL_CONF_INITIATOR)
        // replaces = "": no need replace initiator from 2nd conf member
        eventChain(conf_uri, EH_CONF_NEXT, E_NULL, peer, "", "");
    else if(calls->getGlobalMode() == GLOBAL_IDLE)
        eventChain(peer, EH_DA, E_NULL, "", "", "");
    // for CONF_MEMBER do nothing

    // callView.resizeColumnsToContents();
}

void Dialog::slotRxConfView(QModelIndex)
{
    rxBssView.clearSelection();
}

void Dialog::slotRxBssView(QModelIndex)
{
    rxConfView.clearSelection();
}

void Dialog::slotTxConfView(QModelIndex)
{
    txBssView.clearSelection();
}

void Dialog::slotTxBssView(QModelIndex)
{
    txConfView.clearSelection();
}

void Dialog::slotGroupLabelView(QModelIndex index)
{
    qDebug() << "slotGroupLabelView:index" << index;
    int id = index.row();
    current_bss = MIN_BSS+id;
    current_cli = MIN_CLI+id;
    refreshGroup();
}

void Dialog::slotSubMenuView(QModelIndex index)
{
    // ToDo: if index = "Ok" - depended on current_menu UPDATE tables
    qDebug() << "slotSubMenuView:index" << index;
    updateGroup(index.column());

}

void Dialog::slotMenuGroupButton(int id)
{
    current_menu = static_cast<e_menu>(id-1);
    refreshGroup();
}

void Dialog::slotButtonSet()
{
    // indexList will contain list of indexes selected cells
    /*
    QItemSelectionModel* sm = groupView.selectionModel();
    QModelIndexList indexList = sm->selectedIndexes();
    for(int i = 0; i < indexList.size(); i++)
    {
        qDebug() << i << indexList.at(i);
    }
    */
}

void Dialog::slotButtonReset()
{
    // test selection from program, not mouse
    QItemSelectionModel* sm = groupView.selectionModel();
    QModelIndex index = groupView.model()->index(0, 0);
    sm->select(index, QItemSelectionModel::Clear);
}

Dialog::~Dialog()
{
    qDebug() << "Dialog deleteting...";
    //delete menuBox;
    delete calls;
    delete ui;
}
