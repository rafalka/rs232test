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

#include "MainWindow.h"

#include <QAction>
#include <QInputDialog>

#include "serialsetupdialog.h"
#include "MacrosEditDialog.h"

#include "qextserialenumerator.h"
#include "qextserialport.h"
#include "qhexedit.h"

#include "debug.h"

//static const char*


// ******************************************************************************** C L A S S: MainWindow

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logFile(0)
    , current_intput_mode_idx(-1)
    , current_output_mode_idx(-1)
{
    setupUi();

    prepareInputModes();

    updateConfig(CONF_OP_READ);

    // Override config by commandline
    if (appcmdline.editMode>=0 )    current_intput_mode_idx = appcmdline.editMode;
    if (appcmdline.displayMode>=0 ) current_output_mode_idx = appcmdline.displayMode;
    if (appcmdline.selPort) selPortName = appcmdline.selPort;

    // allocate convertes
    display_convs[OUTMODE_HEX]   = QBinStrConvCollection::getConv(QBinStrConvCollection::CONV_HEX);
    display_convs[OUTMODE_ASCII] = QBinStrConvCollection::getConv(QBinStrConvCollection::CONV_ASCII);
    if (current_output_mode_idx<0||current_output_mode_idx>=__OUTMODES_CNT) current_output_mode_idx = 0;

    createDisplayModeMenu();

    createDevicesList();


    selectInputMode( static_cast<input_modes_t>(current_intput_mode_idx) );
    updateInputModeHistoryMenu();
    updateInputModeMacrosMenu();

    createInputModeMenu();

    port = new QextSerialPort(QextSerialPort::EventDriven,this);

    ASSERT_ALWAYS( connect(port, SIGNAL(readyRead()),    SLOT(onReadyRead()) ) );
    ASSERT_ALWAYS( connect(port, SIGNAL(bytesWritten(qint64)), SLOT(onBytesWritten(qint64)) ) );

    updateUiAccordingToPortState(false,"NONE");

    ASSERT_ALWAYS( connect(ui->binaryEditor, SIGNAL(inputChanged()),               SLOT(onInputChanged() ) ) );
    ASSERT_ALWAYS( connect(ui->binaryEditor, SIGNAL(inputPosChanged(int)),         SLOT(onInputPosChanged(int) ) ) );
    ASSERT_ALWAYS( connect(ui->binaryEditor, SIGNAL(inputSizeChanged(int)),        SLOT(onInputSizeChanged(int) ) ) );
    ASSERT_ALWAYS( connect(ui->binaryEditor, SIGNAL(overwriteModeChanged(bool)),   SLOT(onInputOverwriteModeChanged(bool) ) ) );


    if (appcmdline.logFileName)
    {
        openLogFile(
                    ( appcmdline.logFileName == CONF_MARK_USE_DEFAULT_STR)
                    ? getDefaultLogFileName()  // If file name not provided, then generate default one
                    : QString(appcmdline.logFileName)
                    );
    }


    //QString("%1 (%2)").arg(windowTitle()).arg(phase) ;
    log(QString("%1: %2. Author: %3").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()).arg(QCoreApplication::organizationName())
        ,"#666699");

    if (appcmdline.doConnect)
    {
        //we'll connect automatically, but only if port provided by user if available
        //(we'll check this by checking whether any item in devices list was selected in createDevicesList();
        int idx;
        if ( (idx=ui->devicesComboBox->currentIndex())>=0 &&
             (ui->devicesComboBox->itemData(idx).toString()==selPortName)
           )
        {
            on_connectBtn_clicked();
        }
    }
    //
}

MainWindow::~MainWindow()
{
    if (port->isOpen() )
    {
        port->close();
    }

    updateConfig(CONF_OP_WRITE);

    closeLogFile();

    delete ui;
    delete port;
    port = NULL;

}

void  MainWindow::setupUi()
{
    ui->setupUi(this);

    /// Setting up input status bar
    inputStatus = new QStatusBar();
    inputStatus->setObjectName(QString::fromUtf8("inputStatus"));
    inputStatus->setSizeGripEnabled(false);

    // Position Label
    QLabel* lbSumName = new QLabel();
    lbSumName->setText(tr("FCS:"));
    inputStatus->addPermanentWidget(lbSumName);
    lbSum = new QLabel();
    lbSum->setFrameShape(QFrame::Panel);
    lbSum->setFrameShadow(QFrame::Sunken);
    lbSum->setMinimumWidth(30);
    inputStatus->addPermanentWidget(lbSum);

    // Position Label
    QLabel* lbAddressName = new QLabel();
    lbAddressName->setText(tr("Pos:"));
    inputStatus->addPermanentWidget(lbAddressName);
    lbAddress = new QLabel();
    lbAddress->setFrameShape(QFrame::Panel);
    lbAddress->setFrameShadow(QFrame::Sunken);
    lbAddress->setMinimumWidth(70);
    inputStatus->addPermanentWidget(lbAddress);

    // Size Label
    QLabel* lbSizeName = new QLabel();
    lbSizeName->setText(tr("Size:"));
    inputStatus->addPermanentWidget(lbSizeName);
    lbSize = new QLabel();
    lbSize->setFrameShape(QFrame::Panel);
    lbSize->setFrameShadow(QFrame::Sunken);
    lbSize->setMinimumWidth(70);
    inputStatus->addPermanentWidget(lbSize);

    // Overwrite Mode Label
    QLabel* lbOverwriteModeName = new QLabel();
    lbOverwriteModeName->setText(tr("Mode:"));
    inputStatus->addPermanentWidget(lbOverwriteModeName);
    lbOverwriteMode = new QLabel();
    lbOverwriteMode->setFrameShape(QFrame::Panel);
    lbOverwriteMode->setFrameShadow(QFrame::Sunken);
    lbOverwriteMode->setMinimumWidth(30);
    inputStatus->addPermanentWidget(lbOverwriteMode);


    ui->inputEditLayout->addWidget(inputStatus,ui->inputEditLayout->rowCount()+1,0,1,ui->inputEditLayout->columnCount()-1);

    ui->splitter->setStretchFactor ( 0, 10 ); // Edit Part
    ui->splitter->setStretchFactor ( 1, 80 ); // Display part

    ui->historyBtn->setMenu(&input_mode_history_menu);

    ui->macrosBtn->setMenu(&input_mode_macros_menu);


}

QString MainWindow::getDefaultLogFileName()
{
    return QString("%1_%2_%3.log.html")
            .arg(QCoreApplication::applicationName())
            .arg(QDate::currentDate().toString("yyyy-MM-dd"))
            .arg(QTime::currentTime().toString("hh.mm.ss.zzz")
                 );
}

void MainWindow::openLogFile(const QString logFileName)
{
    logFile = new QFile(logFileName);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        logError(QString("Failed to create log file: %1").arg(logFileName));
        delete  logFile;
        logFile = 0;
        return;
    }
    //Write HTML preamble
    QString htmlHead;
    htmlHead = QString(
            "<html lang=\"en\">\n"
            "<head>\n"
            "<title>%1 log created at %2 %3</title>\n" )
            .arg(QCoreApplication::applicationName())
            .arg(QDate::currentDate().toString("yyyy-MM-dd"))
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz")) ;

    if ( logFile->write(htmlHead.toLatin1())<0 )
    {
        logError(QString("Failed to write to log file: %1").arg(logFileName));
        delete  logFile;
        logFile = 0;
    }
}

void MainWindow::closeLogFile()
{
    if (logFile)
    {
        // put HTML ending
        logFile->write("\n</body></html>\n");
        delete  logFile;
        logFile = 0;
    }
}


void MainWindow::prepareInputModes()
{
    InputMode* inm;

    inm = &input_modes[INMODE_HEX];
    inm->name = "HEX";
    inm->input_converter = NULL;
    inm->display_converter = QBinStrConvCollection::getConv(QBinStrConvCollection::CONV_HEX);
    inm->edit_mode = BinaryEditor::INMODE_HEX;
    inm->editor = ui->binaryEditor;

    inm = &input_modes[INMODE_ASCII];
    inm->name = "ASCII";
    inm->input_converter   = QStrBinConvCollection::getConv(QStrBinConvCollection::CONV_ASCII);
    inm->display_converter = QBinStrConvCollection::getConv(QBinStrConvCollection::CONV_ASCII);
    inm->edit_mode = BinaryEditor::INMODE_TEXT;
    inm->editor = ui->binaryEditor;

    inm = &input_modes[INMODE_CSTR];
    inm->name = "C-like string";
    inm->input_converter   = QStrBinConvCollection::getConv(QStrBinConvCollection::CONV_CSTR);
    inm->display_converter = QBinStrConvCollection::getConv(QBinStrConvCollection::CONV_CSTR);
    inm->edit_mode = BinaryEditor::INMODE_TEXT;
    inm->editor = ui->binaryEditor;

}

void MainWindow::selectInputMode(MainWindow::input_modes_t mode)
{
    //current_intput_mode_idx
    switch (mode)
    {
    case INMODE_HEX:
        ui->binaryEditor->setEditMode(BinaryEditor::INMODE_HEX);
        break;
    case INMODE_ASCII:
    case INMODE_CSTR:
        ui->binaryEditor->setTextModeCoverters(
                    input_modes[mode].input_converter   ,
                    input_modes[mode].display_converter );
        ui->binaryEditor->setEditMode(BinaryEditor::INMODE_TEXT);
    default:
        mode = INMODE_ASCII;
    }

    if ( current_intput_mode_idx != static_cast<int>(mode) )
    {
        current_intput_mode_idx = static_cast<int>(mode);
        updateInputModeHistoryMenu();
        updateInputModeMacrosMenu();
    }

    input_modes[current_intput_mode_idx].editor->refreshInput();
}


void MainWindow::updateUiAccordingToPortState(bool is_open, const QString& portName)
{
    static const QString defWndName = QCoreApplication::applicationName(); //windowTitle();

    setWindowTitle( QString("%1 : %2 : %3").arg(defWndName).arg(portName).arg((is_open)?"Connected":"Disconnected") );

    ui->connectBtn->setChecked(is_open);
    ui->devicesComboBox->setEnabled(! is_open );
    ui->sendBtn->setEnabled(is_open);
    //ui->setupBtn->setEnabled(! is_open);

}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::createInputModeMenu()
{
    QAction*      act;
    int           cnt;
    QActionGroup* menuGroup = new QActionGroup(this);
#if 0
    QStrBinConv*  conv;

    for (cnt=0; cnt<QStrBinConvCollection::getCount(); cnt++)
    {
        conv = QStrBinConvCollection::getConv(cnt);
        act = inputModeMenu.addAction(QString(conv->getName()),
                                      this,
                                      SLOT(inputModeTriggered()));
        act->setCheckable(true);
        act->setData(cnt);
        menuGroup->addAction(act);
        //TODO: Ugly: set default converted in menu creation routine :-(
        if (!cnt)
        {
            act->setChecked(true);
            setInputConv( conv );
        }
        else
        {
            QStrBinConvCollection::disposeConv(conv);
        }
    }
#else
    for (cnt=0; cnt<static_cast<int>(__INMODES_CNT); cnt++)
    {
        act = inputModeMenu.addAction(input_modes[cnt].name,
                                      this,
                                      SLOT(inputModeTriggered()));
        act->setCheckable(true);
        act->setData(cnt);
        //TODO: Ugly: set default converted in menu creation routine :-(
        if (cnt==current_intput_mode_idx)
        {
            act->setChecked(true);
        }
        menuGroup->addAction(act);
    }
#endif
    ui->inputModeMenuBtn->setMenu(&inputModeMenu);
}

void MainWindow::createDevicesList()
{
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    ui->devicesComboBox->clear();
    int selIndex = -1;
    for (int i = 0; i < ports.size(); i++) {
        ui->devicesComboBox->addItem(QString("%1\t- %2").arg(ports.at(i).portName).arg(ports.at(i).friendName),ports.at(i).portName);

        if (selPortName==ports.at(i).portName)
        {
            selIndex = i;
        }
    }
    if (selIndex>=0)
    {
        ui->devicesComboBox->setCurrentIndex(selIndex);
    }
}

void MainWindow::updateConfig(cfg_operations_t operation)
{
    AutoCfg_QString::doCfg(  operation, &selPortName,    "PortName");

    // Input mode settings
    InputMode* inm;

    appconfig->beginGroup("InputMode");
    AutoCfg_int::doCfg(operation, &current_intput_mode_idx, "SelInputMode" );
    AutoCfg_int::doCfg(operation, &current_output_mode_idx, "SelOutputMode" );

    for (int cnt=0; cnt<static_cast<int>(__INMODES_CNT); cnt++)
    {
        inm = &input_modes[cnt];
        appconfig->beginGroup(inm->name);

        AutoCfg_QVariantList::doCfg(operation, &inm->history, "EditorHistory" );
        AutoCfg_QVariantMap::doCfg( operation, &inm->macros,  "EditorMacros"  );

        appconfig->endGroup();
    }
    appconfig->endGroup();



}

void MainWindow::displayErrMsg(const QString &msg)
{
     ui->outputTextEdit->appendPlainText(msg);
}


void MainWindow::createDisplayModeMenu()
{
    QAction*      act;
    QActionGroup* menuGroup = new QActionGroup(this);

    //Very ugly but quick - we creating menu "from hand"

    act = displayModeMenu.addAction("HEX", this, SLOT(displayModeTriggered()));
    act->setCheckable(true);
    act->setData(OUTMODE_HEX);
    if(current_output_mode_idx==OUTMODE_HEX) act->setChecked(true);
    menuGroup->addAction(act);

    act = displayModeMenu.addAction("ASCII", this, SLOT(displayModeTriggered()));
    act->setCheckable(true);
    act->setData(OUTMODE_ASCII);
    if(current_output_mode_idx==OUTMODE_ASCII) act->setChecked(true);
    menuGroup->addAction(act);

#if 0
    int           cnt;
    QBinStrConv*  conv;
    for (cnt=0; cnt<QBinStrConvCollection::getCount(); cnt++)
    {
        conv = QBinStrConvCollection::getConv(cnt);
        act = displayModeMenu.addAction(QString(conv->getName()),
                                      this,
                                      SLOT(displayModeTriggered()));
        act->setCheckable(true);
        act->setData(cnt);
        menuGroup->addAction(act);
        //TODO: Ugly: set default converted in menu creation routine :-(
        if (!cnt)
        {
            act->setChecked(true);
            displayConv = conv;
        }
        else
        {
            QBinStrConvCollection::disposeConv(conv);
        }
    }
#endif
    ui->displayModeMenuBtn->setMenu(&displayModeMenu);
}

void MainWindow::inputModeTriggered()
{
    QAction* who = static_cast<QAction*>( QObject::sender () );
    if (who)
    {
        ui->statusBar->showMessage(QString("Input mode set to: %1").arg(who->text()),2000);
#if 0
        setInputConv( QStrBinConvCollection::getConv(who->data().toInt()) );
#else
        selectInputMode(static_cast<input_modes_t>(who->data().toInt()) );
#endif
    }
}

void MainWindow::displayModeTriggered()
{
    QAction* who = static_cast<QAction*>( QObject::sender () );
    if (who)
    {
        ui->statusBar->showMessage(QString("Display mode set to: %1").arg(who->text()),2000);

        current_output_mode_idx = who->data().toInt();
    }

}

void MainWindow::inputHistoryTriggered()
{
    InputMode& inm = input_modes[current_intput_mode_idx];
    if (! inm.isValid() ) return;

    QAction* who = static_cast<QAction*>( QObject::sender () );
    if (who)
    {
        QByteArray arr = who->data().toByteArray();
        inm.setEditorData( arr );
    }
}

void MainWindow::inputMacrosAddTriggered()
{
    QAction* who = static_cast<QAction*>( QObject::sender () );
    if (who)
    {

        InputMode& inm = input_modes[current_intput_mode_idx];
        QByteArray buf;
        if ( inm.isValid()  )
        {
             buf = inm.getEditorData();
        }

        if ( buf.size() )
        {
            bool ok;
            QString text = QInputDialog::getText(this,
                                 tr("Add macro..."),
                                 tr("Macro name:"),
                                 QLineEdit::Normal,
                                 inm.display_converter->convert( buf, QBinStrConv::PLAIN_TEXT, QBinStrConv::OUTB_SIMPLE ),
                                 &ok);

            if (ok && !text.isEmpty())
            {
                inm.addMacroEntry( text, buf );
                updateInputModeMacrosMenu();
            }

        }
        else
        {
            ui->statusBar->showMessage(
                        tr("Cannot add macro: editor is empty")
                        );
        }


    }
}

void MainWindow::inputMacrosEditTriggered()
{

    QAction* who = static_cast<QAction*>( QObject::sender () );
    if (who)
    {
        InputMode&       inm = input_modes[current_intput_mode_idx];
        if ( MacrosEditDialog::Execute(inm.macros,inm.edit_mode,this) )
        {
             updateInputModeMacrosMenu();
        }
    }
}


void MainWindow::onInputChanged()
{
    const QByteArray* buf=NULL;
    QString          str;
    InputMode&       inm = input_modes[current_intput_mode_idx];

    if ( inm.isValid() )
    {
        buf = inm.getEditorConstData();
    }
    if ( buf && buf->size() )
    {
        str = QString("0x%1").arg( calcFCS(*buf) ,2,16,QChar('0') );
    }

    lbSum->setText( str );
}

void MainWindow::onInputSizeChanged(int size)
{
    lbSize->setText( QString("%1 / 0x%2").arg(size).arg(size,2,16,QChar('0')) );
}

void MainWindow::onInputPosChanged(int pos)
{
    lbAddress->setText( QString("%1 / 0x%2").arg(pos).arg(pos,2,16,QChar('0')) );
}

void MainWindow::onInputOverwriteModeChanged(bool is_ovr_mode)
{
    lbOverwriteMode->setText(is_ovr_mode ? "OVR" : "INS" );
}

void MainWindow::updateInputModeHistoryMenu()
{

    input_mode_history_menu.clear();

    InputMode& inm = input_modes[current_intput_mode_idx];
    if (! inm.isValid() ) return;

    QAction*      act;
    QVariantList *storage = &inm.history;
    QByteArray    arr;

    for (int cnt = 0; cnt < storage->size(); ++cnt)
    {
        const QVariant& n = storage->at(cnt);
        if ( ! n.canConvert(QVariant::ByteArray) ) continue;

        arr = n.toByteArray();
        act = input_mode_history_menu.addAction(
                    inm.display_converter->convert( arr, QBinStrConv::PLAIN_TEXT, QBinStrConv::OUTB_SIMPLE ),
                    this,
                    SLOT(inputHistoryTriggered())
              );
        act->setData( arr );

    }

}

void MainWindow::updateInputModeMacrosMenu()
{
    input_mode_macros_menu.clear();

    InputMode& inm = input_modes[current_intput_mode_idx];
    if (! inm.isValid() ) return;


    QAction*      act;
    QVariantMap&  storage = inm.macros;
    QByteArray    arr;
    QVariantMap::const_iterator i = storage.constBegin();

    input_mode_macros_menu.addAction(
                tr("Add current editor content to macros..."),
                this,
                SLOT( inputMacrosAddTriggered() )
             );
    input_mode_macros_menu.addAction(
                tr("Edit macros..."),
                this,
                SLOT( inputMacrosEditTriggered() )
             );
    input_mode_macros_menu.addSeparator();

    // Make list of stored macros
    while (i != storage.constEnd())
    {
        const QVariant& n = i.value();
        if (  n.canConvert(QVariant::ByteArray) )
        {
            arr = n.toByteArray();
            act = input_mode_macros_menu.addAction(
                        i.key(),
                        this,
                        SLOT( inputHistoryTriggered() )
                  );
            act->setData( arr );
        }

        ++i;


    }

}



void MainWindow::onReadyRead()
{
    QByteArray buf = port->readAll();
    logOpBlue(QString("Read %1 bytes").arg(buf.size()));
    QBinStrConv* displayConv = currentDisplayConv();
    if (displayConv)
    {
        outHtml( displayConv->convert(buf, QBinStrConv::HTML) );
    }
    else
    {
        logError("no display format set");
    }
    //outPlainText("\n");
}

void MainWindow::onBytesWritten(qint64 bytes)
{
    logOpGray(QString("&lt;&lt;&lt; %1 bytes sent\n").arg(bytes));
    /*
    if (bytes>0)
    {
        if(size!=buf.size()) buf.resize(bytes);
        outPlainText( displayConv->convert(buf) );
    }
    */

}

qint64 MainWindow::sendData(const QByteArray &data )
{
    if (! port->isOpen() ) return 0;

    qint64      size = data.size();
    qint64      sent;
    const char* ptr = data.constData();

    while(size>0)
    {
        sent = port->write(ptr,size);
        if (!sent) break;
        ptr  += sent;
        size -= sent;
    }
    return data.size()-size;
}

void MainWindow::on_sendBtn_clicked()
{
#if 0
    if (inputConv && displayConv)
    {
        QString    str = ui->inputCombo->currentText();
        QByteArray buf;
        int        erridx;


        switch (inputConv->convert(str,&buf,&erridx))
        {
            case QStrBinConv::VALID:
                {
                    logOpGray(QString("&gt;&gt;&gt; Sending %1 bytes...\n").arg(buf.size()));
                    outHtml( displayConv->convert(buf,QBinStrConv::HTML) );

                    if ( ui->inputCombo->findText(str) < 0 )
                    {
                        ui->inputCombo->addItem(str);
                    }

                    sendData(buf);
                }
                break;
            case QStrBinConv::MIGHT_VALID:
                QApplication::beep();
                break;
            case QStrBinConv::INVALID:
                QApplication::beep();
                break;
            default:
                break;
        }

    }
#else
    InputMode* inm = currentInputMode();
    if ( inm )
    {
        QByteArray buf = inm->getEditorData();
        if ( buf.size() )
        {
            logOpGray(QString("&gt;&gt;&gt; Sending %1 bytes...").arg(buf.size()));
            if (currentDisplayConv() )
            outHtml( currentDisplayConv()->convert(buf,QBinStrConv::HTML) );

            inm->addHistoryEntry(&buf);

            sendData(buf);

            updateInputModeHistoryMenu();
        }

    }
#endif

}


void MainWindow::on_connectBtn_clicked()
{
    if (port->isOpen() )
    {
        port->close();
    }
    else if (ui->devicesComboBox->currentIndex()>=0)
    {
        port->setPortName( ui->devicesComboBox->itemData(ui->devicesComboBox->currentIndex()).toString() );
        if (! port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
        {
            displayErrMsg(QString("Cannot open port: %1. Error: %2")
                            .arg(port->portName())
                            .arg(port->errorString()) );
        }
    }

    updateUiAccordingToPortState(port->isOpen(),port->portName() );

}

void MainWindow::on_devicesComboBox_onShowPopup()
{
    createDevicesList();
}





void MainWindow::on_devicesComboBox_activated(int index)
{
    selPortName = ui->devicesComboBox->itemData(index).toString();
    if (! port->isOpen() && (index>=0) )
    {
        port->setPortName( selPortName );
    }
}

void MainWindow::on_setupBtn_clicked()
{
    PortSettings port_conf = port->getPortSetting();

    if ( SerialSetupDialog::Excecute(port_conf,port->portName(),this) )
    {
        port->setPortSettings(port_conf);
    }

}


// ******************************************************************************** C L A S S: QConvValidator

QValidator::State QConvValidator::validate( QString & input, int & pos ) const
{
    // Q_ASSERT( conv );

    (void) pos; // Unused so far...

    if ( conv )
    {
        switch( conv->validate(input) )
        {
            case QStrBinConv::INVALID:      return QValidator::Invalid;
            case QStrBinConv::MIGHT_VALID:  return QValidator::Intermediate;
            case QStrBinConv::VALID:        return QValidator::Acceptable;
            case QStrBinConv::BUF_TO_SMALL: return QValidator::Invalid;
            default:
                Q_ASSERT(!"Unsupported QStrBinConv::");
        }
    }
    return QValidator::Acceptable;

}

