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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QValidator>
#include <QComboBox>
#include <QTime>
#include <QLabel>
#include <QVariantList>
#include <QVariantMap>
#include <QFile>
#include <QString>

#include "appconfig.h"

#include "strbinconv.h"
#include "qextserialport.h"
#include "InputHistoryList.h"

#include "BinaryEditor.h"

extern void displayErrorMessage(const QString& err);

namespace Ui {
class MainWindow;
}

// ******************************************************************************** C L A S S:

// ******************************************************************************** C L A S S:  QMyComboBox
class QMyComboBox : public QComboBox
{
    Q_OBJECT
public:
    QMyComboBox(QWidget* parrent = NULL):QComboBox(parrent) {}
    virtual void showPopup() { emit onShowPopup(); QComboBox::showPopup(); }
signals:
    void onShowPopup();

};


#include "ui_MainWindow.h"


// ******************************************************************************** C L A S S:  QMyComboBox
class QConvValidator: public QValidator
{
    Q_OBJECT
public:
    explicit QConvValidator(QObject * parent = 0)
        : QValidator( parent )
        , conv( 0 )
    {}

    ~QConvValidator() { /*if (conv) conv->detach();*/ }
private:
    Q_DISABLE_COPY(QConvValidator)
private:
    QStrBinConv*    conv;
public:
    void setConv(QStrBinConv*  conv)
    {
        //if (this->conv) this->conv->detach();
        this->conv = conv;
        //if (this->conv) this->conv->attach();

    }
    QStrBinConv*               getConv() { return conv; }
    virtual QValidator::State  validate ( QString & input, int & pos ) const;
};




// ******************************************************************************** C L A S S:  InputMode
class InputMode
{
protected:
    QString              name;

    BinaryEditor*               editor;
    BinaryEditor::input_modes_t edit_mode;

    QBinStrConv*         converter;

    QVariantList         history;
    QVariantMap          macros;

public:
    bool       isValid()       { return (editor && converter); }
    QByteArray        getEditorData() { return (editor) ? editor->getInputData(): QByteArray() ; }
    const QByteArray* getEditorConstData() { return (editor) ? &editor->getInputConstData(): NULL ; }
    void       setEditorData(QByteArray& data) { if (editor) editor->setInputData(data); }

    void       addHistoryEntry(QByteArray* entry) { InputHistoryList::Add(&history,entry); }
    void       addMacroEntry(QString& name, QByteArray& entry)
    {
        macros[name] = entry;
    }


    friend class MainWindow;
};


// ******************************************************************************** C L A S S:  OutputMode
/* NOT USED
class OutputMode
{
protected:
    QString              name;
    QBinStrConv*         converter;

public:
    bool       isValid()       { return (converter); }

    friend class MainWindow;
};
*/

// ******************************************************************************** C L A S S:  MainWindow
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    typedef enum {
        INMODE_HEX,
        INMODE_ASCII,

        __INMODES_CNT
    } input_modes_t;

    typedef enum {
        OUTMODE_HEX,
        OUTMODE_ASCII,

        __OUTMODES_CNT
    } output_modes_t;


protected:
    QConvValidator inputValidator;
    QextSerialPort* port;

    qint64 sendData(const QByteArray &data );

    void updateUiAccordingToPortState(bool is_open, const QString& portName);

    void changeEvent(QEvent *e);
    void createInputModeMenu();
    void createDisplayModeMenu();
    void createDevicesList();

private:
    Ui::MainWindow *ui;
    void  setupUi();
    QStatusBar* inputStatus;
    QLabel* lbAddress;
    QLabel* lbSize;
    QLabel* lbOverwriteMode;
    QLabel* lbSum;

    QFile*  logFile;

    QString  getDefaultLogFileName();
    void     openLogFile(const QString logFileName);
    void     closeLogFile();

    QMenu inputModeMenu;
    QMenu displayModeMenu;
    QMenu input_mode_history_menu;
    QMenu input_mode_macros_menu;

    QString       selPortName;

    void        connectInputEditor(InputEditorAbstract* editor);
    InputMode   input_modes[__INMODES_CNT];
    void        prepareInputModes();
    void        selectInputMode(input_modes_t mode);
    InputMode*  currentInputMode() { InputMode* inm = &input_modes[current_intput_mode_idx];
                                     return ( inm->isValid() ) ? inm : NULL; }
    int         current_intput_mode_idx;


    QBinStrConv* display_convs[__OUTMODES_CNT];
    QBinStrConv* currentDisplayConv() { return display_convs[current_output_mode_idx]; }
    int         current_output_mode_idx;

    InputEditorAbstract* editor;

    void          updateConfig(cfg_operations_t operation);
public:
    void log(const QString& msg, const char* color = "black", const char* fmt="i")
    {
        outHtml(QString("<br /><%3><font color=\"%2\">%1</font></%3>\n")
              .arg(msg).arg(color).arg(fmt));
    }

    void logOperation(const QString& msg, const char* color = "black", const char* fmt="i")
    {
        outHtml(QString("<br /><%3><font color=\"%2\">[%4] %1</font></%3>\n")
               .arg(msg).arg(color).arg(fmt)
               .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));
    }
    void logOpBlue(const QString& msg) { logOperation(msg,"blue"); }
    void logOpGray(const QString& msg) { logOperation(msg,"gray"); }
    void logOpGreen(const QString& msg) { logOperation(msg,"green"); }

    void logError(const QString& msg) { outHtml(QString("<b><font color=\"red\">ERROR: %1</font></b>").arg(msg)); }
    void displayErrMsg(const QString& msg);

    void outPlainText(const QString& msg)
    {
        if (logFile) logFile->write( msg.toLatin1() );
        ui->outputTextEdit->appendPlainText(msg);
    }
    void outHtml(const QString& msg)
    {
        if (logFile){
            logFile->write( msg.toLatin1() );
            logFile->write("<br />\n");
        }
        ui->outputTextEdit->appendHtml(msg);
    }

public slots:
    void inputModeTriggered();
    void displayModeTriggered();
    void inputHistoryTriggered();

    void inputMacrosAddTriggered();
    void inputMacrosEditTriggered();
    //void inputMacrosTriggered(); - used inputHistoryTriggered instead


    void   onInputChanged();
    void   onInputSizeChanged(int size);
    void   onInputPosChanged(int pos);
    void   onInputOverwriteModeChanged(bool is_ovr_mode);

private slots:
    void   updateInputModeHistoryMenu();
    void   updateInputModeMacrosMenu();

private slots:
    void on_sendBtn_clicked();
    void on_connectBtn_clicked();
    void on_devicesComboBox_onShowPopup();
    void onReadyRead();
    void onBytesWritten( qint64 bytes );
    void on_devicesComboBox_activated(int index);
    void on_setupBtn_clicked();
};

extern MainWindow w;

#endif // MAINWINDOW_H
