/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "widget.h"
#include "dialog.h"

RadioFrame::RadioFrame(QWidget *parent) :
    QFrame(parent)
{
    setFixedSize(RCELL_W, RCELL_H);
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("background-color:black;");

    /*
    init left side widget, consist on freq, status, mode and led labels on gridlayout
    */

    // left frame setup
    frameLeft = new QFrame;
    frameLeft->setFixedSize(RCELL_W_LFRAME, RCELL_H_ICON);
    frameLeft->setFrameShape(QFrame::Box);
    layoutLeft = new QGridLayout;
    frameLeft->setLayout(layoutLeft);
    layoutLeft->setMargin(0);

    // labels
    freqLabel = new QLabel;
    freqLabel->setFixedSize(RCELL_W_FREQ, RCELL_H_FREQ);

    statusLabel = new QLabel;
    statusLabel->setFixedSize(RCELL_W_FREQ, RCELL_H_ICON-RCELL_H_FREQ);

    modeLabel = new QLabel;
    modeLabel->setFixedSize(RCELL_W_LFRAME-RCELL_W_FREQ, RCELL_H_FREQ);

    ledLabel = new QLabel;
    ledLabel->setFixedSize(RCELL_W_LFRAME-RCELL_W_FREQ,  RCELL_H_ICON-RCELL_H_FREQ);

    // pixmap to draw circle
    pixmap = new QPixmap(30, 30);
    pixmap->fill(Qt::transparent);
    painter = new QPainter;
    pen = new QPen;
    brush = new QBrush(Qt::SolidPattern);

    layoutLeft->addWidget(freqLabel, 0, 0, Qt::AlignLeft);
    layoutLeft->addWidget(statusLabel, 1, 0, Qt::AlignLeft);
    layoutLeft->addWidget(modeLabel, 0, 1, Qt::AlignLeft);
    layoutLeft->addWidget(ledLabel, 1, 1, Qt::AlignLeft);

    // init right side
    iconLabel = new QLabel;
    iconLabel->setFixedSize(RCELL_W_ICON, RCELL_H_ICON);

    // init all RadioWidget
    layout = new QHBoxLayout;
    setLayout(layout);

    layout->addWidget(frameLeft, Qt::AlignTop);
    layout->addWidget(iconLabel, Qt::AlignTop);
    layout->setSpacing(2);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
}

void RadioFrame::setSelect(bool sel)
{
    QString styleFrame;
    QString styleFreq;
    QString styleStatus;
    QString styleIcon;
    if(sel)
    {
        styleFrame =  ".QFrame {border: 1px solid black; background-color: green; margin: 0px; padding: 0px; spacing: 0px;}";
        styleFreq =  "QLabel {"
                "background-color : rgb(214, 214, 214); color black: ;"
                "}";
        styleStatus =  "QLabel {"
                "background-color : rgb(190, 190, 190); color black: ;"
                "}";
        styleIcon =  "QLabel {border: 1px solid black; background-color: white; margin: 0px; padding: 0px; spacing: 0px;}";
    }
    else
    {
        styleFrame =  ".QFrame {border: 1px solid rgb(214, 214, 214); background-color: green; margin: 0px; padding: 0px; spacing: 0px;}";
        styleFreq =  "QLabel {"
                "background-color : black; color : rgb(214, 214, 214);"
                "}";
        styleStatus =  "QLabel {"
                "background-color : blue; color : rgb(214, 214, 214);"
                "}";
        styleIcon =  "QLabel {border: 1px solid rgb(214, 214, 214); background-color: black; margin: 0px; padding: 0px; spacing: 0px;}";
    }

    frameLeft->setStyleSheet(styleFrame);
    freqLabel->setStyleSheet(styleFreq);
    statusLabel->setStyleSheet(styleStatus);
    modeLabel->setStyleSheet(styleStatus);
    ledLabel->setStyleSheet(styleFreq);
    iconLabel->setStyleSheet(styleIcon);
}

void RadioFrame::setFields(QString list)
{
    bool config = list.section(';', 0, 0).toInt();
    QString freq = list.section(';', 1, 1);
    QString name = list.section(';', 2, 2);
    e_freq mode = static_cast<e_freq>(list.section(';', 3, 3).toInt());
    Qt::GlobalColor led = static_cast<Qt::GlobalColor>(list.section(';', 4, 4).toInt());

    freqLabel->setText(freq);
    statusLabel->setText(name);
    modeLabel->setText("C");

    pen->setColor(led);
    brush->setColor(led);
    painter->begin(pixmap);
    painter->setBrush(*brush);
    painter->setPen(*pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->drawEllipse(QPoint(8, 8), 4, 4);
    ledLabel->setPixmap(*pixmap);
    painter->end();

    if(config)
        iconLabel->setText("<img src=':/radio/res/icons/settings.png' />");
    else
    {
        switch(mode)
        {
        case F_OFF:
            iconLabel->setText("<img src=':/radio/res/icons/radio-wireless-signal-icone-5919-32.png' />");
            break;
        case F_RX:
            iconLabel->setText("<img src=':/radio/res/icons/No_active_Rx-64.png' />");
            break;
        case F_RXTX:
            iconLabel->setText("<img src=':/radio/res/icons/No_active_RxTx-64.png' />");
            break;
        }
    }
}

RadioFrame::~RadioFrame()
{
    delete freqLabel;
    delete statusLabel;
    delete modeLabel;
    delete iconLabel;
    delete layout;
    delete frameLeft;
    delete pen;
    delete brush;
    delete pixmap;
    delete painter;
}


