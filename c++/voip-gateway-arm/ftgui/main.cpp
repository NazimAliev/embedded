/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <QtGui/QApplication>
#include "version.h"
#include "dialog.h"

QString log_id;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if(argc != 4)
    {
        log("Usage: ftgui 1xx 6xx db", LOG_ERR);
        exit(1);
    }
    int my_cwp = atoi(argv[1]);
    log_id = QString("HMI#%1 ").arg(my_cwp);
    int focus = atoi(argv[2]);
    QString dbname = argv[3];
    QFile fdb(dbname);
    if(!fdb.exists())
    {
        log(QString("DB %1 not found").arg(dbname), LOG_ERR);
        exit(1);
    }

    QSqlDatabase db;
    // sqlite initialisation
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbname);
    bool ok = db.open();
    if(!ok)
    {
        log(QString("error open DB %1").arg(dbname), LOG_ERR);
        exit(1);
    }
    QSqlQuery query;
    Q_ASSERT(query.prepare("PRAGMA foreign_keys=ON"));
    Q_ASSERT(query.exec());

    // test DB has valid format (is sqlite database)
    ok = query.prepare("SELECT * FROM book");
    if(!ok)
    {
        log(QString("DB %1 has invalid format").arg(dbname), LOG_ERR);
        exit(1);
    }
    Q_ASSERT(query.exec());

    Dialog w;
    // primary key in book starts from 100
    w.setup(my_cwp, focus);
    w.show();
    
    return a.exec();
}

void log(QString msg, e_log log)
{
    QString out;
    switch(log)
    {
    case LOG_SIP:
        out = "\t";
        out += C_G;
        break;
    case LOG_ON:
        out = "\t\t";
        out += C_M;
        break;
    case LOG_UDP:
        out = C_B;
        break;
    case LOG_INFO:
        out = C_H;
        break;
    case LOG_WARN:
        out = C_Y;
        out += "WARNING: ";
        break;
    case LOG_ERR:
        out = C_R;
        out += "ERROR: ";
        break;
    case LOG_OTHER:
        out = "\t";
        break;
    default:
        out = C_R;
        out += "Log Error - undefined log type: ";
        out += QString("%1").arg(log);
        break;
    }
    out += log_id;
    out += msg += C_;
    fprintf(stderr, "%s\n", QString(out).toAscii().constData());
}

