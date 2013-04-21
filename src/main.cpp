/******************************************************************************
 * @file
 *
 * @brief
 *
 * @date     01-03-2013
 * @author   Rafal Kukla
 ******************************************************************************
 *       Copyright (C) 2013 Rafal Kukla ( rkdevel AT gmail DOT com )
 *        This file is a part of rs232test project and is released
 *      under the terms of the license contained in the file LICENSE
 ******************************************************************************
 */

#include <cstdio>

/** BOOST testing
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#ifdef POSIX
#include <termios.h>
#endif

*/

#include <QString>
#include <QApplication>
#include <QVariantMap>
#include <QMessageBox>
#include <QScopedPointer>
#include <iostream>

#include "cpputils.h"

#include "appconfig.h"
#include "parseopt.h"
#include "printopt.h"

#include "MainWindow.h"
#include "strbinconv.h"

MainWindow*          mainWnd=NULL;
QSettings*           appconfig=NULL;
AppCommandLineParams appcmdline;

static const char* phase = "BETA (" __DATE__ ")";

void displayErrorMessage(const QString& err)
{
    QApplication::beep();

    if (mainWnd)
        mainWnd->displayErrMsg(QString("Error: %1\n").arg(err));
    else
    {
        //fprintf(stderr,"Error: %s\n",err.toLatin1().data());
        QMessageBox::critical(NULL,
                              QString(QObject::tr("%1. Error")).arg( QCoreApplication::applicationName() ),
                              QString("Error: %1\n").arg(err));
    }
}
void displayMessage(const QString& msg)
{

    if (mainWnd)
        mainWnd->displayErrMsg(msg);
    else
    {
        fprintf(stderr, "%s\n",msg.toLatin1().data());

        QMessageBox msgBox(QMessageBox::NoIcon,
                           QCoreApplication::applicationName(),
                           msg,
                           QMessageBox::Ok,
                           0,
                           Qt::Window);
        QFont font;
        font.setFamily(QString::fromUtf8("Consolas"));
        msgBox.setFont( font );
        msgBox.exec();

/*        fprintf(stderr,"%s\n",msg.toLatin1().data());
        QMessageBox::information(NULL,
                              QCoreApplication::applicationName() ,
                              msg);
                              */

    }
}

extern "C" void output_error_msg(const char* msg)
{
  displayErrorMessage(msg);
}

bool AppCommandLineParams::parseCommandLine(int argc, char *argv[])
{
    return parse_opt(argc,argv,options,POPT_IGNORE_CASE)>=0;
}
opt_val_listitem_t   AppCommandLineParams::inmode_list[] =
{
    {MainWindow::INMODE_HEX,   "hex|h",       "HEX",     0 },
    {MainWindow::INMODE_ASCII, "ascii|a",     "ASCII",   0 },
    {MainWindow::INMODE_CSTR,  "cstr|c",      "C-like string",   0 },
    {0,           NULL,          NULL,                0 } // List terminator
};

opt_val_listitem_t   AppCommandLineParams::outmode_list[] =
{
    {MainWindow::OUTMODE_HEX,   "hex|h",       "HEX",     0 },
    {MainWindow::OUTMODE_ASCII, "ascii|a",     "ASCII",   0 },
    {MainWindow::OUTMODE_CSTR,  "cstr|c",      "C-like string",   0 },
    {0,           NULL,          NULL,                0 } // List terminator
};

AppCommandLineParams::AppCommandLineParams()
    : selPort(NULL)
    , configFileName(NULL)
    , displayMode(-1)
    , editMode(-1)
    , helpRequeted(0)
    , doConnect(0)
{

    opt_defs_t*  op=options;
    PUT_BOOL_OPT( op, 'h', "help",                      &helpRequeted,                   "Show command line options");
    PUT_BOOL_OPT( op, 'c', "connect",                   &doConnect,                      "Automatically connect to port after starting");
    PUT_CHAR_OPT( op,  0 , "config",     "cfgfile",     &configFileName,  NULL,          "Configuration file. If not defined then Registry will be used");
    PUT_CHAR_OPT( op, 'p', "port",       "port name",   &selPort,         NULL,          "Port file name that should be selected");
    //PUT_OPT_DEF_CHAR(pOptDef,pShortOpt,pcLongOpt,PO_REQUIRED_ARG,pcArgName,ppcVal,pcOptDef,NULL,NULL,1,0,pcDesc)
    PUT_OPT_DEF_CHAR( op, 'l', "log",  PO_OPTIONAL_ARG, "log file",    &logFileName, CONF_MARK_USE_DEFAULT_STR ,NULL,NULL,1,0,
                      "Dump output to log file with provided name or default file name if [log file] parameter is not provided");
    PUT_LIST_OPT( op, 'i', "inmode",     "mode",        &editMode,     inmode_list,  NULL, "Edit mode");
    PUT_LIST_OPT( op, 'o', "outmode",    "mode",        &displayMode,  outmode_list, NULL, "Display mode");
    PUT_END_OPT(  op );
}

QString AppCommandLineParams::getCommandLineHelpString()
{
    int                        x  =60;
#if 0
//#ifdef WIN32
    /** TODO: Enable getting console width for non-Windows */
    CONSOLE_SCREEN_BUFFER_INFO ci;
#endif /*WIN32*/
    VFILE                      file;

    vfopendm(&file);

#if 0
//#ifdef WIN32
    /** TODO: Enable getting console width for non-Windows */
    if (GetConsoleScreenBufferInfo( (HANDLE) _fileno(stderr),&ci)) x=ci.dwSize.X;
#endif /*WIN32*/
    vfprint_opts_desc(&file,options,x,NULL);
    QString str = file.ptr;

    vfclose(&file);

    return str;
}

int main(int argc, char *argv[])
{
    //printf("rs232test started...\n");

    QCoreApplication::setOrganizationName("Rafal Kukla");
    QCoreApplication::setOrganizationDomain("rafalkukla.com");
    QCoreApplication::setApplicationName("Rs232Test2");
    QCoreApplication::setApplicationVersion(phase);
    QApplication a(argc, argv);

    if (! appcmdline.parseCommandLine(argc,argv) )
    {
        displayErrorMessage("Application command line parsing error.");
        return -1;
    }

    if (appcmdline.helpRequeted)
    {

        displayMessage(QString("%1: %2\nAuthor: %3\n\n%4")
                       .arg(QCoreApplication::applicationName())
                       .arg(QCoreApplication::applicationVersion())
                       .arg(QCoreApplication::organizationName())
                       .arg(appcmdline.getCommandLineHelpString()) );

        return 1;
    }

    QScopedPointer<QSettings*, QScopedSubPointerDeleter<QSettings*> > settings_deleter(&appconfig);

    if (appcmdline.configFileName)
    {
        // Custom config file provided
        appconfig = new QSettings(QString(appcmdline.configFileName),QSettings::IniFormat );
    }
    else
    {
        // if not, then use native format and storage...
        appconfig = new QSettings();
    }


    MainWindow w;
    mainWnd = &w;

    w.show();
    return a.exec();
}
