/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <QtCore/QCoreApplication>
#include <stdio.h>
#include "version.h"
#include "console.h"

// log level for log() function
// 0 - no log
// 3 - log all

#define LOG_LEVEL 3

// true if program works as radio gateway - use radio table instead of book
bool GW;
// true if program works as conference focus - 6xx key in book table
bool FOCUS;

QString log_id;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString squery;
    QString dbname;

    GW = false;
    FOCUS = false;

    if(argc == 4 && QString(argv[1]) == "-c")
    {
        // ftsip/bin/ftsip -c 100 ftsip/bin/global.db
        log_id = QString("SIP#%1 ").arg(argv[2]);
        squery = QString("SELECT * FROM book WHERE key=%1").
                arg(atoi(argv[2]));
        dbname = argv[3];
        log("CWP mode", LOG_OTHER);
    }
    else if(argc == 4 && QString(argv[1]) == "-f")
    {
        // ftsip/bin/ftsip -f 600 ftsip/bin/global.db
        FOCUS = true;
        log_id = QString("FOCUS#%1 ").arg(argv[2]);
        squery = QString("SELECT * FROM book WHERE key=%1").
                arg(atoi(argv[2]));
        dbname = argv[3];
        log("Focus mode", LOG_OTHER);
    }
    else if(argc == 5 && QString(argv[1]) == "-r")
    {
        // ftsip/bin/ftsip -r 300 200 ftsip/bin/global.db
        GW = true;
        log_id = QString("GW#%1:%2 ").arg(argv[2]).arg(argv[3]);
        squery = QString("SELECT * FROM radio, freq WHERE radio.id=%1 AND radio.ref_freq=freq.key AND radio.ref_freq=%2").
                arg(atoi(argv[2])).arg(atoi(argv[3]));
        dbname = argv[4];
        log("GW mode", LOG_OTHER);
    }
    else
    {
        log("Usage:\n"
			"CWP mode: ftsip -c 1xx db\n"
			"FOCUS mode: ftsip -f 6xx db\n"
			"Radio GW mode: ftsip -r 3xx 2xx db\n", LOG_ERR);
        exit(1);
    }

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
    ok = query.prepare("PRAGMA foreign_keys=ON");
    Q_ASSERT(ok);
    ok = query.exec();
    Q_ASSERT(ok);

    // test DB has valid format (is sqlite database)
    ok = query.prepare("SELECT * FROM book");
    if(!ok)
    {
        log(QString("DB %1 has invalid format").arg(dbname), LOG_ERR);
        exit(1);
    }
    ok = query.exec();
    Q_ASSERT(ok);

	// TODO: create console dynamicaly
    Console c;
    if(c.initConfig(squery) != 0)
        exit(1);
    c.cwp = atoi(argv[2]);
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

