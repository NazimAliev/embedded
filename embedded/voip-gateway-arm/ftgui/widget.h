/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef WIDGET_H
#define WIDGET_H

#define RCELL_W 200
#define RCELL_H 98
#define RCELL_SP 12
#define RCELL_W_ICON 70
#define RCELL_W_LFRAME RCELL_W-2*RCELL_SP-RCELL_W_ICON
#define RCELL_W_FREQ 80
#define RCELL_H_ICON RCELL_H-2*RCELL_SP
#define RCELL_H_FREQ 48

#include <QWidget>
#include "model.h"

class RadioFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RadioFrame(QWidget *parent = 0);
    ~RadioFrame();

    void setSelect(bool sel);
    void setFields(QString list);
    
signals:
    
public slots:

private:
    QFrame* frameLeft;
    QGridLayout* layoutLeft;
    QLabel* freqLabel;
    QLabel* statusLabel;
    QLabel* modeLabel;
    QLabel* ledLabel;
    QLabel* iconLabel;
    QHBoxLayout* layout;
    QPixmap* pixmap;
    QPainter* painter;
    QPen* pen;
    QBrush* brush;
};


#endif // WIDGET_H
