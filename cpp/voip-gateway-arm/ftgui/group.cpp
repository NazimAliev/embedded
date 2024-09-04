/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

void Dialog::refreshGroup()
{
    // called after changed current_menu, current_freq
    bookWidget->hide();
    radioWidget->hide();
    rxConfWidget->hide();
    txConfWidget->hide();
    groupWidget->hide();
    subMenuWidget->hide();

    QSqlQuery query;
    QString freq_label;
    query.prepare("SELECT freq, name FROM freq WHERE key=?");
    query.bindValue(0, current_freq);
    query.exec();
    query.first();
    freq_label = "[" + query.value(0).toString() + " MHz ";
    e_freq mode = freqModel.getMode(current_freq);
    if(mode == F_OFF)
        freq_label += "OFF";
    if(mode == F_RX)
        freq_label += "Rx";
    if(mode == F_RXTX)
        freq_label += "RxTx";
    freq_label += "]";
    // level logic
    // setLevel(R_DISABLED) means isOn>0, i.e. enabled or selected will be bold
    // setLevel(R_ENABLED) means isOn>1, i.e. only selected will be bold
    switch(current_menu)
    {
    case G_ADDR:
        bookWidget->show();
        break;
    case G_RADIO:
        radioModel.setLevel(R_DISABLED);
        radioModel.setQuery(Radio.arg(current_freq));
        radioTitle.setText(freq_label + " Enabled Radios");
        radioWidget->show();
        subMenuModel.setMode(0);
        subMenuView.resizeColumnsToContents();
        subMenuWidget->show();
        break;
    case G_RX:
        rxConfModel.setLevel(R_ENABLED);
        rxConfModel.setQuery(Radio_rx.arg(current_freq));
        rxBssModel.setQuery(Bss.arg(current_freq));
        rxBssView.resizeColumnsToContents();
        rxTitle.setText(freq_label + " Rx Config");
        rxConfWidget->show();
        subMenuModel.setMode(1);
        subMenuView.resizeColumnsToContents();
        subMenuWidget->show();
        break;
    case G_TX:
        txConfModel.setLevel(R_ENABLED);
        txConfModel.setQuery(Radio_tx.arg(current_freq));
        txBssModel.setQuery(Cli.arg(current_freq));
        txTitle.setText(freq_label + " Tx Config");
        txBssView.resizeColumnsToContents();
        txConfWidget->show();
        subMenuModel.setMode(1);
        subMenuView.resizeColumnsToContents();
        subMenuWidget->show();
        break;
    case G_BSS:
        groupLabelModel.setQuery(Label_bss.arg(current_freq));
        groupModel.setLevel(R_DISABLED);
        // child query in RadioChildModel, not in RadioModel!
        groupModel.setQuery(Label_bss_radio.arg(current_bss),
                            Radio_rx.arg(current_freq));
        groupTitle.setText(freq_label + " BSS[" + QString("%1").arg(current_bss) + "]");
        groupLabelView.resizeColumnsToContents();
        groupWidget->show();
        subMenuModel.setMode(2);
        subMenuView.resizeColumnsToContents();
        //subMenuView.selectColumn(3);
        subMenuWidget->show();
        break;
    case G_CLI:
        groupLabelModel.setQuery(Label_cli.arg(current_freq));
        groupModel.setLevel(R_DISABLED);
        // child query in RadioChildModel, not in RadioModel!
        groupModel.setQuery(Label_cli_radio.arg(current_cli),
                            Radio_tx.arg(current_freq));
        groupTitle.setText(freq_label + " Climax Groups");
        groupLabelView.resizeColumnsToContents();
        groupWidget->show();
        subMenuModel.setMode(2);
        subMenuView.resizeColumnsToContents();
        //subMenuView.selectColumn(3);
        subMenuWidget->show();
        break;
    default:
        break;
        ;
    }

}

void Dialog::updateGroup(int submenu)
{
    // called after changing selections and click submenu "Ok..."
    switch(current_menu)
    {
    case G_ADDR:
        // no submenu
        break;
    case G_RADIO:
        // set available radios
        // Ok, X, Space, MG, GG, Space, Off, Rx, RxTx
        switch(submenu)
        {
        case 0:
            // Ok
            // find selected radios
            // send selected to setData()
            radioModel.setUpdate(Update_radio);
            updateSelections(&radioModel, &radioView);
            break;
        case 1:
            // X
            radioView.clearSelection();
            break;
        case 2:
        case 5:
            // Space
            break;
        case 3:
            // MG
            break;
        case 4:
            // GG
            break;
        case 6:
            // Off
            freqModel.setMode(current_freq, F_OFF);
            break;
        case 7:
            // Rx
            // ToDo: test if Rx mode radio present - at least one
            freqModel.setMode(current_freq, F_RX);
            break;
        case 8:
            // RxTx
            // ToDo: test if Tx mode radio present - at least one
            freqModel.setMode(current_freq, F_RXTX);
            break;
        default:
            break;
        }
        break;
    case G_RX:
        switch(submenu)
        {
        case 0:
            // Ok
            rxConfModel.setUpdate(Update_rx);
            updateSelections(&rxConfModel, &rxConfView);
            rxBssModel.setUpdate(Update_bss);
            updateSelections(&rxBssModel, &rxBssView);
            break;
        case 1:
            // X
            rxConfView.clearSelection();
            rxBssView.clearSelection();
            break;
        default:
            break;
        }
        break;
    case G_TX:
        switch(submenu)
        {
        case 0:
            // Ok
            txConfModel.setUpdate(Update_tx);
            updateSelections(&txConfModel, &txConfView);
            txBssModel.setUpdate(Update_cli);
            updateSelections(&txBssModel, &txBssView);
            break;
        case 1:
            // X
            txConfView.clearSelection();
            txBssView.clearSelection();
            break;
        default:
            break;
        }
        break;
    case G_BSS:
        switch(submenu)
        {
        case 0:
            // Ok
            groupModel.setUpdate(Insert_bss.arg(current_bss), Delete_bss.arg(current_bss));
            updateSelections(&groupModel, &groupView);
            break;
        case 1:
            // X
            groupView.clearSelection();
            break;
        case 2:
            // Space
            break;
        case 3:
            // Del
            break;
        default:
            break;
        }
        break;
    case G_CLI:
        switch(submenu)
        {
        case 0:
            // Ok
            groupModel.setUpdate(Insert_cli.arg(current_cli), Delete_cli.arg(current_cli));
            updateSelections(&groupModel, &groupView);
            break;
        case 1:
            // X
            groupView.clearSelection();
            break;
        default:
            break;
        }
    }
    refreshGroup();
    return;
}

// for selected in view cells send 1 in model setData(index), otherwise 0
void Dialog::updateSelections(QSqlQueryModel* model, QTableView* view)
{
    QModelIndex index;
    QModelIndexList indexList;

    QItemSelectionModel* sm = view->selectionModel();
    indexList = sm->selectedIndexes();
    if(indexList.size() == 0)
    {
        log("Nothing selected - group can't be empty. Use remove group instead", LOG_WARN);
        //return;
    }

    QVariant value;
    for(int row = 0; row < model->rowCount(); row++)
        for(int col = 0; col < model->columnCount(); col++)
        {
            index = model->index(row, col);
            if(sm->isSelected(index))
            {
                value = QVariant(1);
            }
            else
            {
                value = QVariant(0);
            }
            model->setData(index, value);
        }

    return;
}

