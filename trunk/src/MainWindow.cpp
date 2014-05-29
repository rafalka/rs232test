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
#include <QtSerialPort/QSerialPortInfo>

#include "MacrosEditDialog.h"

//#include "qextserialenumerator.h"
//#include "qextserialport.h"
#include "qhexedit.h"

#include "debug.h"

//static const char*

static const SerialSetupDialog::PortSettings defaultPortSettings = {115200,QSerialPort::Data8, QSerialPort::NoParity, QSerialPort::OneStop,QSerialPort::NoFlowControl, 250};
// ******************************************************************************** C L A S S: MainWindow

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logFile(0)
    , portSettings(defaultPortSettings)
    , current_intput_mode_idx(-1)
    , current_output_mode_idx(-1)
    , outopt(OUTOPT_SHOW_INPUT | OUTOPT_SHOW_OUT_INFO)
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
    display_convs[OUTMODE_CSTR]  = QBinStrConvCollection::getConv(QBinStrConvCollection::CONV_CSTR);
    if (current_output_mode_idx<0||current_output_mode_idx>=__OUTMODES_CNT) current_output_mode_idx = 0;

    createDisplayModeMenu();
    createDisplayOptionsMenu();

    createDevicesList();


    selectInputMode( static_cast<input_modes_t>(current_intput_mode_idx) );
    updateInputModeHistoryMenu();
    updateInputModeMacrosMenu();

    createInputModeMenu();

    _port = new QSerialPort(this);

    ASSERT_ALWAYS( connect(_port, SIGNAL(error(QSerialPort::SerialPortError)),    SLOT(onSerialPortError(QSerialPort::SerialPortError)) ) );

    ASSERT_ALWAYS( connect(_port, SIGNAL(readyRead()),    SLOT(onReadyRead()) ) );
    ASSERT_ALWAYS( connect(_port, SIGNAL(bytesWritten(qint64)), SLOT(onBytesWritten(qint64)) ) );
    //ASSERT_ALWAYS( connect(_port, SIGNAL(dataTerminalReadyChanged(bool)),    SLOT(onLineChanged(bool)) ) );
    //ASSERT_ALWAYS( connect(_port, SIGNAL(requestToSendChanged(bool)),        SLOT(onLineChanged(bool)) ) );
    ASSERT_ALWAYS( connect(_port, SIGNAL(pinoutSignalsChanged(QSerialPort::PinoutSignals)),        SLOT(onSerialLinesChanged(QSerialPort::PinoutSignals)) ) );


    updateUiAccordingToPortState(false,"NONE");
    updateUiAccordingToPinoutSignals(0);

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
    if (_port->isOpen() )
    {
        _port->close();
    }

    updateConfig(CONF_OP_WRITE);

    closeLogFile();

    delete ui;
    delete _port;
    _port = NULL;

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
        break;
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

    ui->dtrBtn->setEnabled( is_open );
    ui->rtsBtn->setEnabled( is_open );

    //ui->setupBtn->setEnabled( is_open);

}

void MainWindow::updateUiAccordingToPinoutSignals(QSerialPort::PinoutSignals pinoutSignals)
{
    ui->dtrBtn->setChecked( pinoutSignals & QSerialPort::DataTerminalReadySignal );
    ui->rtsBtn->setChecked( pinoutSignals & QSerialPort::RequestToSendSignal );



    ui->ctsLbl->setAutoFillBackground( pinoutSignals & QSerialPort::ClearToSendSignal );
    ui->dsrLbl->setAutoFillBackground( pinoutSignals & QSerialPort::DataSetReadySignal );
    ui->rngLbl->setAutoFillBackground( pinoutSignals & QSerialPort::RingIndicatorSignal );
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
    int           cnt;

    for (cnt=0; cnt<static_cast<int>(__INMODES_CNT); cnt++)
    {
        ui->InputModeCombo->addItem(input_modes[cnt].name,cnt);
    }
    ui->InputModeCombo->setCurrentIndex(current_intput_mode_idx);
}

void MainWindow::createDisplayModeMenu()
{
    int           cnt;
    for (cnt=0; cnt<static_cast<int>(__OUTMODES_CNT); cnt++)
    {
        ui->DisplayModeCombo->addItem(display_convs[cnt]->getName(),cnt);
    }
}

void MainWindow::addDisplayOptToMenu(QMenu* menu, QString name, output_options_t opt)
{
    QAction*      act;
    act = menu->addAction(name, this, SLOT(displayOptionsTriggered()) );
    act->setData(opt);
    act->setCheckable(true);
    act->setChecked( outopt & opt );
}

void MainWindow::createDisplayOptionsMenu()
{
    QMenu* menu = new QMenu();
    addDisplayOptToMenu(menu, tr("Display data sent do port"), OUTOPT_SHOW_INPUT);
    addDisplayOptToMenu(menu, tr("Display received data info"), OUTOPT_SHOW_OUT_INFO);
    ui->dsplOptionsMenuBtn->setMenu(menu);
}

void MainWindow::createDevicesList()
{
    //QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QSerialPortInfo       info;
    QString               portName;

    ui->devicesComboBox->clear();
    int selIndex = -1;
    for (int i = 0; i < ports.size(); i++) {
        info     = ports.at(i);
        portName = info.portName();
        ui->devicesComboBox->addItem(QString("%1\t- %2").arg( portName ).arg(info.description()),portName);

        if (selPortName==portName)
        {
            selIndex = i;
        }
    }
    if (selIndex>=0)
    {
        ui->devicesComboBox->setCurrentIndex(selIndex);
    }
}

void MainWindow::updatePortConfig(cfg_operations_t operation)
{
#define RW_PORTSETTINGS_FIELD(_type_, _name_) AutoCfg<_type_,convVarAsIntTo<_type_> >::doCfg(operation, &portSettings._name_, #_name_)
    RW_PORTSETTINGS_FIELD(qint32,                   BaudRate);
    RW_PORTSETTINGS_FIELD(QSerialPort::DataBits,    DataBits);
    RW_PORTSETTINGS_FIELD(QSerialPort::Parity,      Parity);
    RW_PORTSETTINGS_FIELD(QSerialPort::StopBits,    StopBits);
    RW_PORTSETTINGS_FIELD(QSerialPort::FlowControl, FlowControl);
    RW_PORTSETTINGS_FIELD(long,                     Timeout_Millisec);
    //AutoCfg<qint32,                convVarAsIntTo<qint32>                >::doCfg(operation, portSettings.BaudRate, "BaudRate");
    //AutoCfg<QSerialPort::DataBits, convVarAsIntTo<QSerialPort::DataBits> >::doCfg(operation, portSettings.DataBits, "DataBits");
 #undef RW_PORTSETTINGS_FIELD
}

void MainWindow::updateConfig(cfg_operations_t operation)
{
    AutoCfg_QString::doCfg(  operation, &selPortName,    "PortName");
    appconfig->beginGroup("PortSettings");
    updatePortConfig(operation);
    appconfig->endGroup();
    // Input mode settings
    InputMode* inm;

    AutoCfg_int::doCfg(operation, &outopt, "DisplayFlags" );

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

void MainWindow::displayOptionsTriggered()
{
    QAction* who = static_cast<QAction*>( QObject::sender () );
    if (who)
    {
        int opt = who->data().toInt();
        if (who->isChecked() ) {
            outopt |= opt;
        }
        else
        {
            outopt &= ~opt;
        }
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
    //logOpBlue("Read Event...");
    QByteArray buf;// = port->readAll();

    int maxlen = _port->bytesAvailable();
    if (maxlen<=0) return;

    if (maxlen<8192) maxlen = 8192; // If we want to use timeouts, we must try read more than is in buffer.
    buf.resize(maxlen);

    maxlen = _port->read(buf.data(), maxlen );
    buf.resize(maxlen);
    if (outopt & OUTOPT_SHOW_OUT_INFO)
    {
        logOpBlue(QString("Read %1 bytes").arg( maxlen ));
    }
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
    if (outopt & OUTOPT_SHOW_INPUT)
    {
        logOpGray(QString("&lt;&lt;&lt; %1 bytes sent\n").arg(bytes));
    }
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
    if (! _port->isOpen() ) return 0;

    qint64      size = data.size();
    qint64      sent;
    const char* ptr = data.constData();

    while(size>0)
    {
        sent = _port->write(ptr,size);
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
            if (outopt & OUTOPT_SHOW_INPUT)
            {
                logOpGray(QString("&gt;&gt;&gt; Sending %1 bytes...").arg(buf.size()));
                if (currentDisplayConv() )
                outHtml( currentDisplayConv()->convert(buf,QBinStrConv::HTML) );
            }

            inm->addHistoryEntry(&buf);

            sendData(buf);

            updateInputModeHistoryMenu();
        }

    }
#endif

}




void MainWindow::on_InputModeCombo_activated(int index)
{
    selectInputMode( static_cast<input_modes_t>(ui->InputModeCombo->itemData(index).toInt()) );
}

void MainWindow::on_DisplayModeCombo_activated(int index)
{
    //selectInputMode( static_cast<input_modes_t>(ui->InputModeCombo->itemData(index).toInt()) );
     current_output_mode_idx = ui->DisplayModeCombo->itemData(index).toInt();
}


void MainWindow::on_connectBtn_clicked()
{
    if (_port->isOpen() )
    {
        _port->close();
    }
    else if (ui->devicesComboBox->currentIndex()>=0)
    {
        _port->setPortName( ui->devicesComboBox->itemData(ui->devicesComboBox->currentIndex()).toString() );
        if (! _port->open(QIODevice::ReadWrite /*| QIODevice::Unbuffered*/) )
        {
            displayErrMsg(QString("Cannot open port: %1. Error: %2")
                            .arg(_port->portName())
                            .arg(_port->errorString()) );
        }
        else
        {
            setPortSetting(_port, portSettings);
            updateUiAccordingToPinoutSignals(_port->pinoutSignals());
        }
    }

    updateUiAccordingToPortState(_port->isOpen(),_port->portName() );

}

void MainWindow::on_devicesComboBox_onShowPopup()
{
    createDevicesList();
}


void MainWindow::on_devicesComboBox_activated(int index)
{
    selPortName = ui->devicesComboBox->itemData(index).toString();
    if (! _port->isOpen() && (index>=0) )
    {
        _port->setPortName( selPortName );
    }
}

void MainWindow::getPortSetting(QSerialPort* port, SerialSetupDialog::PortSettings& settings)
{
    if (port)
    {
        settings.BaudRate  = (QSerialPort::BaudRate) port->baudRate();
        settings.DataBits = port->dataBits();
        settings.FlowControl = port->flowControl();
        settings.Parity = port->parity();
        settings.StopBits = port->stopBits();
        settings.Timeout_Millisec = port->getTimeout();
    }
}

void MainWindow::setPortSetting(QSerialPort* port, const SerialSetupDialog::PortSettings& settings)
{
    if (port)
    {
        port->setBaudRate(settings.BaudRate);
        port->setDataBits(settings.DataBits);
        port->setFlowControl(settings.FlowControl);
        port->setParity(settings.Parity);
        port->setStopBits(settings.StopBits);
        port->setTimeout(settings.Timeout_Millisec);
    }
}

void MainWindow::on_setupBtn_clicked()
{
    if (_port->isOpen()) {
        getPortSetting(_port,portSettings);
    }

    if ( SerialSetupDialog::Excecute(portSettings,_port->portName(),this) )
    {
        if (_port->isOpen()) {
            setPortSetting(_port, portSettings);
        }
    }
}

void MainWindow::on_dtrBtn_clicked(bool checked)
{
    if (_port->isOpen())
        _port->setDataTerminalReady(checked);
}

void MainWindow::on_rtsBtn_clicked(bool checked)
{
    if (_port->isOpen())
        _port->setRequestToSend(checked);

}


void MainWindow::on_dsplClearBtn_clicked()
{
    if ( ui->outputTextEdit->document()->isEmpty() ) return;


    if ( QMessageBox::question(
                this,
                tr("Clear output area"),
                tr("Output area is not empty\nDo you want to clear entire conent?"),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel
            ) == QMessageBox::Yes )
    {
        ui->outputTextEdit->clear();
    }
}


void MainWindow::on_dsplSaveBtn_clicked()
{
    if ( ui->outputTextEdit->document()->isEmpty() ) return;

    QFileDialog dialog(this);
    QStringList filters;

    /*filters << "Text files (*.txt)"
            << "HTML files (*.htm *.html)"
            << "Any files (*)";

    filters << "text/plain"
            << "text/html"
            << "application/octet-stream";
    dialog.setMimeTypeFilters(filters);

    //if (dialog.exec()) return;
    */

    QString sel_filters;
    QString file_name = QFileDialog::getSaveFileName(this,
                             tr("Save output area"),
                             QString(),
                             tr("Text Files (*.txt);;HTML files (*.html *.htm);;All files (*.*)"),
                             &sel_filters
                          );
    if ( file_name.isEmpty() ) return;

    QFile  file(file_name);
    if (! file.open(QIODevice::WriteOnly) )
    {
        displayErrorMessage(QString("Cannot open for writing file: %1").arg(file_name) );
        return;
    }
    logOpGreen(file_name);
    logOpGreen(sel_filters);

    QByteArray content = (sel_filters.startsWith("HTML"))
            ? ui->outputTextEdit->document()->toHtml().toUtf8()
            : ui->outputTextEdit->document()->toPlainText().toLocal8Bit() ;

    file.write(content);

}


const char* MainWindow::getSerialPortErrorString(QSerialPort::SerialPortError error)
{
    const char* str;

    switch (error) {
    case QSerialPort::NoError:                   str = "No Error"; break;
    case QSerialPort::DeviceNotFoundError:       str = "Device Not Found"; break;
    case QSerialPort::PermissionError:           str = "Permission Error"; break;
    case QSerialPort::OpenError:                 str = "Open Error"; break;
    case QSerialPort::ParityError:               str = "Parity Error"; break;
    case QSerialPort::FramingError:              str = "Framing Error"; break;
    case QSerialPort::BreakConditionError:       str = "Break Condition Error"; break;
    case QSerialPort::WriteError:                str = "Write Error"; break;
    case QSerialPort::ReadError:                 str = "Read Error"; break;
    case QSerialPort::ResourceError:             str = "Resource Error"; break;
    case QSerialPort::UnsupportedOperationError: str = "Unsupported Operation"; break;
    case QSerialPort::TimeoutError:              str = "Timeout Error"; break;
    case QSerialPort::NotOpenError:              str = "Not Open Error"; break;
    case QSerialPort::UnknownError:
    default: str = "Unknown Error"; break;
    }

    return str;
}

void MainWindow::onSerialPortError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
    {
        logError(QString("Serial Port Error: ").append(getSerialPortErrorString(error)) );
    }
}

void MainWindow::onLineChanged(bool set)
{
    (void) set;
    if (_port->isOpen())
    {
        updateUiAccordingToPinoutSignals(_port->pinoutSignals());
    }
}

void MainWindow::onSerialLinesChanged(QSerialPort::PinoutSignals signals_mask)
{
    (void) signals_mask;
    if (_port->isOpen())
    {
        updateUiAccordingToPinoutSignals(_port->pinoutSignals());
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

