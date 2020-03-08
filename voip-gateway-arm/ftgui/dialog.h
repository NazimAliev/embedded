/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef DIALOG_H
#define DIALOG_H

#include "model.h"
#include "callmodel.h"
#include "bookmodel.h"
#include "freqmodel.h"
#include "radiomodel.h"

#define C_R QByteArray("\033[0;31m")
#define C_G QByteArray("\033[0;32m")
#define C_Y QByteArray("\033[0;33m")
#define C_B QByteArray("\033[0;34m")
#define C_M QByteArray("\033[0;35m")
#define C_C QByteArray("\033[0;36m")
#define C_H QByteArray("\033[0;37m")
#define C_ QByteArray("\033[0m")

// callModel parameter
#define ROW_DOUBLE 2

typedef enum
{
    LOG_SIP,
    LOG_ON,
    LOG_UDP,
    LOG_INFO,
    LOG_WARN,
    LOG_ERR,
    LOG_OTHER
} e_log;

void log(QString msg, e_log log);

typedef enum
{
    G_ADDR,
    G_RADIO,
    G_RX,
    G_TX,
    G_BSS,
    G_CLI
} e_menu;

class Calls;

namespace Ui {
class Dialog;
}


class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();// log staff - log type (for different colors and permit to show) and log function
    
private:
    Ui::Dialog *ui;
    QWidget* createPanel(QLayout *layout, QLabel *title,
                         QAbstractTableModel *model1, QTableView *view1, QSize size1,
                         QAbstractTableModel *model2 = NULL, QTableView *view2 = NULL, QSize size2 = QSize(0, 0),
                         QAbstractTableModel *model3 = NULL, QTableView *view3 = NULL, QSize size3 = QSize(0, 0));
    int cwp;
    e_menu current_menu;
    int current_freq;
    int current_bss;
    int current_cli;

    // callWidget for ingoing and outgoing calls
    QVBoxLayout callLayout;
    QWidget* callWidget;
    Calls* calls;

    // freqWidget for freq panel
    FreqModel freqModel;
    QTableView freqView;
    QGridLayout freqLayout;
    FreqViewDelegate freqDelegate;
    QWidget* freqWidget;

    // bookWidget for book panel
    BookModel bookModel;
    QTableView bookView;
    QGridLayout bookLayout;
    QWidget* bookWidget;

    // radioWidget for full Rx and Tx
    RadioModel radioModel;
    QTableView radioView;
    QVBoxLayout radioLayout;
    QWidget* radioWidget;
    QLabel radioTitle;

    // rxWidget for Rx config
    RadioModel rxConfModel;
    QTableView rxConfView;
    SimpleModel rxBssModel;
    QTableView rxBssView;
    QVBoxLayout rxConfLayout;
    QWidget* rxConfWidget;
    QLabel rxTitle;

    // txWidget for Tx config
    RadioModel txConfModel;
    QTableView txConfView;
    SimpleModel txBssModel;
    QTableView txBssView;
    SimpleModel txCliModel;
    QTableView txCliView;
    QVBoxLayout txConfLayout;
    QWidget* txConfWidget;
    QLabel txTitle;

    // Widget for groups config
    QTableView groupLabelView;
    LabelModel groupLabelModel;
    RadioChildModel groupModel;
    QTableView groupView;
    QHBoxLayout groupLayout;
    QWidget* groupWidget;
    QLabel groupTitle;

    // submenuWidget
    SubmenuModel subMenuModel;
    QTableView subMenuView;
    QHBoxLayout subMenuLayout;
    QWidget* subMenuWidget;

    // menuWidget for main menu
    QHBoxLayout menuLayout;
    QWidget* menuWidget;
    QButtonGroup menuGroupButton;

    QGridLayout mainLayout;

    // one of group models

    call_t* call;
    e_disp chain_disp;
    e_sip chain_command;
    QString chain_peer;
    e_sip chain_event;
    QString chain_header;
    QString chain_body;
    QString chain_parm;
    QString chain_refer;
    QString chain_replaces;

    void eventChain(QString peer, e_sip command, e_sip event, QString header, QString body, QString parm);
    bool SIP_PROCESS(e_disp disp_check, e_sip event_check, e_disp disp_new, QString text);
    bool HMI_PROCESS(e_disp disp_check, e_sip command_check, e_sip hmi, e_disp disp_new, QString text);
    void deleteConf();
    void hmiRadioEvent(int key);
    bool isIdle();

    QUdpSocket* udpLocalSocketSip;
    // QUdpSocket* udpLocalSocketDsp;
    QHostAddress host_sip_board;
    // QHostAddress host_dsp_board;
    QString ip_atom_board;
    QString ip_sip_board;
    QString ip_dsp_board;
    QString ip_sip_listen;
    QString ip_sip_contact;
    quint16 port_sip_listen;
    quint16 port_control_sip_board_src;
    quint16 port_control_dsp_board_src;
    quint16 port_control_sip_board_dst;
    quint16 port_control_dsp_board_dst;
    QString conf_uri;

    void initUdp();
    void writeDatagramSip(e_sip sip, QString peer, QString header, QString parm, QString text);
    void writeDatagramDebugSip(QString str);
    void writeDatagramDsp(ertp_t ertp);
    void writeDatagramDebugDsp(QString str);

    void refreshGroup();
    void updateGroup(int submenu);
    void updateSelections(QSqlQueryModel *model, QTableView *view);

public slots:
    void slotMenuGroupButton(int id);
    void slotCallView(QModelIndex index);
    void slotFreqView(QModelIndex index);
    void slotBookView(QModelIndex index);
    void slotRxConfView(QModelIndex);
    void slotRxBssView(QModelIndex);
    void slotTxConfView(QModelIndex);
    void slotTxBssView(QModelIndex);
    void slotGroupLabelView(QModelIndex index);
    void slotSubMenuView(QModelIndex index);
    void slotButtonSet();
    void slotButtonReset();
    void slotReadDatagram();
public:
    void setup(int my_cwp, int focus);
};

#endif // DIALOG_H
