/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

QStringList statusList;

void Dialog::setup(int my_cwp, int focus)
{
    cwp = my_cwp;
    setWindowTitle(QString("CWP #%1").arg(cwp));

    {
        statusList.insert(D_OFF, "Idle");
        statusList.insert(D_HOST_DOWN, "Host down! <");
        statusList.insert(D_HOST_DOWN_RADIO, "Radio down! <");
        statusList.insert(D_TRYING_IN, "Trying... >");
        statusList.insert(D_TRYING_OUT, "Host is up <");
        statusList.insert(D_TRYING_RADIO, "Radio is up <");
        statusList.insert(D_BLINK_IN, "Ring... >");
        statusList.insert(D_BLINK_OUT, "Beep... <");
        statusList.insert(D_CALL_IN, "Call in <");
        statusList.insert(D_CALL_OUT, "Call out >");
        statusList.insert(D_SUBS_REFER_TRYING, "Subs refer trying... >");
        statusList.insert(D_SUBS_CONF_INIT_TRYING, "Subs conf init trying... >");
        statusList.insert(D_SUBS_CONF_NEXT_TRYING, "Subs conf next trying... >");
        statusList.insert(D_SUBS_REFER_OK, "Subs refer OK <");
        statusList.insert(D_SUBS_CONF_INIT_OK, "Subs conf init OK <");
        statusList.insert(D_INITIATOR2FOCUS_COMPLETE, "Int to focus OK >");
        statusList.insert(D_SUBS_CONF_NEXT_OK, "Subs conf next OK <");
        statusList.insert(D_UNSUBS_CONF_INITIATOR_TRYING, "Unsubs init trying... >");
        statusList.insert(D_UNSUBS_CONF_MEMBER_TRYING, "Unsubs member trying... >");
        statusList.insert(D_NOTIFY_REFER_TRYING, "Notify refer trying... >");
        statusList.insert(D_NOTIFY_CONF_TRYING, "Notify conf trying... >");
        statusList.insert(D_NOTIFY_REFER_OK, "Notify refer OK <");
        statusList.insert(D_NOTIFY_CONF_OK, "Notify conf OK <");
        statusList.insert(D_REFER_TRYING, "Refer trying... >");
        statusList.insert(D_REFER_ACCEPT, "Refer accepting... <");
        statusList.insert(D_REFER_PROCEED, "Refer proceed... >");
        statusList.insert(D_TERM, "Term");
        statusList.insert(D_HOLD, "Hold");
        statusList.insert(D_CONF, "Conf");
        statusList.insert(D_DIV_ON, "DivOn");
        statusList.insert(D_DIV_OFF, "DivOff");
        statusList.insert(D_CONF_INIT_COMPLETE, "Conf init complete");
        statusList.insert(D_CONF_NEXT_COMPLETE, "Conf next complete");
    }

    /*************
      * CALL Widget *
      ************/

    // Call Widgetmanages dynamicly by Calls class
    callWidget = new QWidget;
    callWidget->setLayout(&callLayout);

    /*************
      * FREQ Widget *
      ************/

    freqModel.setQuery(Freq);
    freqView.hideRow(COL_FREQ_KEY);
    freqView.hideRow(COL_FREQ_NAME);
    freqView.hideRow(COL_FREQ_MODE);
    freqView.setStyleSheet("QTableView{background-color:yellow}");
    freqView.setItemDelegate(&freqDelegate);
    current_freq = freqModel.getFreq(0);
    freqWidget = createPanel(&freqLayout, NULL, &freqModel, &freqView, QSize(RCELL_W*4+8, RCELL_H));
    freqView.resizeColumnsToContents();
    freqView.resizeRowsToContents();

    /*************
      * Book Widget *
      ************/

    bookModel.setQuery(Book);
    bookWidget = createPanel(&bookLayout, NULL, &bookModel, &bookView, QSize(700, 400));
    bookView.resizeColumnToContents(3);
    //freq_label.setText("000.000");
    //freq_label.hide();
    //groupLayout.addWidget(&freq_label, 0, 0);

    /*************
      * RADIO Widget *
      ************/

    radioModel.setLevel(0);
    radioModel.setQuery(Radio.arg(current_freq));
    radioWidget = createPanel(&radioLayout, &radioTitle,
                           &radioModel, &radioView, QSize(700, 400));

    radioView.setSelectionBehavior(QAbstractItemView::SelectItems);
    radioView.setSelectionMode(QAbstractItemView::MultiSelection);
    radioWidget->hide();

    /*************
      * Rx Widget *
      ************/

    rxConfModel.setLevel(1);
    rxConfModel.setQuery(Radio_rx.arg(current_freq));
    rxBssModel.setQuery(Bss.arg(current_freq));

    rxConfWidget = createPanel(&rxConfLayout, &rxTitle,
                           &rxConfModel, &rxConfView, QSize(700, 500),
                           &rxBssModel, &rxBssView, QSize(700, 300));

    rxConfView.setSelectionBehavior(QAbstractItemView::SelectItems);
    rxConfView.setSelectionMode(QAbstractItemView::MultiSelection);
    rxConfWidget->hide();

    /*************
      * Tx Widget *
      ************/

    txConfModel.setLevel(1);
    txConfModel.setQuery(Radio_tx.arg(current_freq));
    txBssModel.setQuery(Bss.arg(current_freq));

    txConfWidget = createPanel(&txConfLayout, &txTitle,
                           &txConfModel, &txConfView, QSize(700, 500),
                           &txBssModel, &txBssView, QSize(700, 300),
                           &txCliModel, &txCliView);

    txConfView.setSelectionBehavior(QAbstractItemView::SelectItems);
    txConfView.setSelectionMode(QAbstractItemView::MultiSelection);
    txConfWidget->hide();


    /*************
      * group Widget *
      ************/

    groupLabelModel.setQuery(Label_bss.arg(current_bss));
    groupModel.setLevel(1);
    groupModel.setQuery(Label_bss_radio.arg(current_bss),
                        Radio.arg(current_freq));
    //groupMenuView.setGeometry(0, 0, 60, 400);
    groupWidget = createPanel(&groupLayout, NULL,
                           &groupLabelModel, &groupLabelView, QSize(150, 400),
                           &groupModel, &groupView, QSize(550, 400));
    //subMenuModel.setMode(2);
    //groupLayout.addWidget(subMenuWidget);

    groupLabelView.setSelectionBehavior(QAbstractItemView::SelectItems);
    groupLabelView.setSelectionMode(QAbstractItemView::SingleSelection);
    groupView.setSelectionBehavior(QAbstractItemView::SelectItems);
    groupView.setSelectionMode(QAbstractItemView::MultiSelection);
    groupWidget->hide();

    /*************
      * sub menu Widget *
      ************/
    subMenuModel.setMode(1);
    subMenuWidget = createPanel(&subMenuLayout, NULL,
                             &subMenuModel, &subMenuView, QSize(700, 30));
    subMenuWidget->hide();

    /*************
      * menu Widget *
      ************/

    QStringList sl;
    sl << "ADDR" << "Radio" << "Rx" << "Tx" << "BSS" << "CLI";
    for (int i = 0; i < sl.size(); ++i)
    {
        QPushButton* pb = new QPushButton(sl.at(i));
        menuLayout.addWidget(pb);
        menuGroupButton.addButton(pb, i+1);
    }
    menuWidget->setLayout(&menuLayout);

    /*************
      * All Widgets *
      ************/

    mainLayout.addWidget(callWidget, 0, 0, 4, 1, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(freqWidget, 0, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(bookWidget, 2, 2, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(radioWidget, 2, 2, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(subMenuWidget, 3, 2, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(rxConfWidget, 2, 2, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(txConfWidget, 2, 2, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(groupWidget, 2, 2, Qt::AlignLeft | Qt::AlignTop);
    mainLayout.addWidget(menuWidget, 4, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);

    mainLayout.setColumnStretch(2,1);
    mainLayout.setRowStretch(2, 1);
    setLayout(&mainLayout);

    connect(&menuGroupButton, SIGNAL(buttonClicked(int)), this, SLOT(slotMenuGroupButton(int)));
    connect(&freqView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotFreqView(QModelIndex)));
    connect(&bookView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotBookView(QModelIndex)));

    // to provide only one selected item between bss and Rx/Tx
    connect(&rxConfView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotRxConfView(QModelIndex)));
    connect(&rxBssView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotRxBssView(QModelIndex)));
    connect(&txConfView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotTxConfView(QModelIndex)));
    connect(&txBssView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotTxBssView(QModelIndex)));

    connect(&groupLabelView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotGroupLabelView(QModelIndex)));
    connect(&subMenuView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotSubMenuView(QModelIndex)));

    log("Setup widgets completed", LOG_OTHER);

    // DB setup
    QSqlQuery query;
    Q_ASSERT(query.prepare("SELECT * FROM book WHERE key=?"));
    query.bindValue(0, cwp);
    Q_ASSERT(query.exec());
    Q_ASSERT(query.first());
    Uri = query.value(BOOK_URI).toString();
    ip_atom_board = query.value(BOOK_IP_ATOM_BOARD).toString();
    ip_sip_board = query.value(BOOK_IP_SIP_BOARD).toString();
    if(ip_sip_board == "")
        ip_sip_board = ip_atom_board;
    ip_dsp_board = query.value(BOOK_IP_DSP_BOARD).toString();
    ip_sip_listen = query.value(BOOK_IP_SIP_LISTEN).toString();
    if(ip_sip_listen == "")
        ip_sip_listen = ip_sip_board;
    ip_sip_contact = query.value(BOOK_IP_SIP_CONTACT).toString();
    if(ip_sip_contact == "")
        ip_sip_contact = ip_sip_board;
    port_sip_listen = query.value(BOOK_PORT_SIP_LISTEN).toUInt();
    port_control_sip_board_src = query.value(BOOK_PORT_CONTROL_SIP_BOARD_SRC).toUInt();
    port_control_dsp_board_src = query.value(BOOK_PORT_CONTROL_DSP_BOARD_SRC).toUInt();
    port_control_sip_board_dst = query.value(BOOK_PORT_CONTROL_SIP_BOARD_DST).toUInt();
    port_control_dsp_board_dst = query.value(BOOK_PORT_CONTROL_DSP_BOARD_DST).toUInt();
    //conf_uri = QString("conf@%1:%2").arg(ip_sip_board).arg(port_sip_listen);

    // get conf focus for this cwp
    Q_ASSERT(query.prepare("SELECT * FROM book WHERE key=?"));
    query.bindValue(0, focus);
    Q_ASSERT(query.exec());
    Q_ASSERT(query.first());
    conf_uri = query.value(BOOK_URI).toString();

    log(QString("uri=%1").arg(Uri), LOG_OTHER);
    log(QString("ip_atom_board=%1").arg(ip_atom_board), LOG_OTHER);
    log(QString("ip_sip_board=%1").arg(ip_sip_board), LOG_OTHER);
    log(QString("ip_dsp_board=%1").arg(ip_dsp_board), LOG_OTHER);
    log(QString("ip_sip_listen=%1").arg(ip_sip_listen), LOG_OTHER);
    log(QString("ip_sip_contact=%1").arg(ip_sip_contact), LOG_OTHER);
    log(QString("port_sip_listen=%1").arg(port_sip_listen), LOG_OTHER);
    log(QString("port_control_sip_board_src=%1").arg(port_control_sip_board_src), LOG_OTHER);
    log(QString("port_control_dsp_board_src=%1").arg(port_control_dsp_board_src), LOG_OTHER);
    log(QString("port_control_sip_board_dst=%1").arg(port_control_sip_board_dst), LOG_OTHER);
    log(QString("port_control_dsp_board_dst=%1").arg(port_control_dsp_board_dst), LOG_OTHER);
    log(QString("conf_uri=%1").arg(conf_uri), LOG_OTHER);

    initUdp();

}

QWidget* Dialog::createPanel(QLayout *layout, QLabel *title,
                               QAbstractTableModel *model1, QTableView *view1, QSize size1,
                               QAbstractTableModel *model2, QTableView *view2, QSize size2,
                               QAbstractTableModel *model3, QTableView *view3, QSize size3)
{
    if(title)
    {
        title->show();
        layout->addWidget(title);
    }
    view1->setModel(model1);
    view1->setFixedSize(size1);
    view1->verticalHeader()->hide();
    view1->horizontalHeader()->hide();
    view1->show();
    layout->addWidget(view1);
    if(model2 && view2)
    {
        view2->setModel(model2);
        view2->setFixedSize(size2);
        view2->verticalHeader()->hide();
        view2->horizontalHeader()->hide();
        view2->show();
        layout->addWidget(view2);
    }
    if(model3 && view3)
    {
        view3->setModel(model3);
        view3->setFixedSize(size3);
        view3->verticalHeader()->hide();
        view3->horizontalHeader()->hide();
        view3->show();
        layout->addWidget(view3);
    }
    QWidget* widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}
