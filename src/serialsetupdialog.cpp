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

#include "serialsetupdialog.h"
#include "ui_serialsetupdialog.h"

SerialSetupDialog::SerialSetupDialog(const QString portName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SerialSetupDialog)
{
    ui->setupUi(this);

    setWindowTitle(portName+" Properties");
    setupUI();
}

SerialSetupDialog::~SerialSetupDialog()
{
    delete ui;
}


void SerialSetupDialog::setupUI()
{
    ui->baudRateBox->addItem("1200",  BAUD1200);
    ui->baudRateBox->addItem("2400",  BAUD2400);
    ui->baudRateBox->addItem("4800",  BAUD4800);
    ui->baudRateBox->addItem("9600",  BAUD9600);
    ui->baudRateBox->addItem("19200", BAUD19200);
    ui->baudRateBox->addItem("38400", BAUD38400);
    ui->baudRateBox->addItem("57600", BAUD57600);
    ui->baudRateBox->addItem("115200", BAUD115200);
    ui->baudRateBox->addItem("230400", BAUD230400);
    ui->baudRateBox->addItem("460800", BAUD460800);
    ui->baudRateBox->addItem("921600", BAUD921600);
    ui->baudRateBox->setCurrentIndex(7);


    ui->parityBox->addItem("None",  PAR_NONE);
    ui->parityBox->addItem("Odd",   PAR_ODD);
    ui->parityBox->addItem("Even",  PAR_EVEN);
#if defined(Q_OS_WIN)
    ui->parityBox->addItem("Mark",  PAR_MARK);
#endif
    ui->parityBox->addItem("Space", PAR_SPACE);
    ui->parityBox->setCurrentIndex(0);


    ui->dataBitsBox->addItem("5", DATA_5);
    ui->dataBitsBox->addItem("6", DATA_6);
    ui->dataBitsBox->addItem("7", DATA_7);
    ui->dataBitsBox->addItem("8", DATA_8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->stopBitsBox->addItem("1",   STOP_1);
#if defined(Q_OS_WIN)
    ui->stopBitsBox->addItem("1.5", STOP_1_5);
#endif
    ui->stopBitsBox->addItem("2",   STOP_2);
    ui->parityBox->setCurrentIndex(0);


    ui->flowCtrlBox->addItem("None",       FLOW_OFF);
    ui->flowCtrlBox->addItem("Hardware",   FLOW_HARDWARE);
    ui->flowCtrlBox->addItem("Xon / Xoff", FLOW_XONXOFF);
    ui->parityBox->setCurrentIndex(0);
}

void SerialSetupDialog::updateGuiToConfig(PortSettings& port_conf)
{
    ui->baudRateBox->setCurrentIndex( ui->baudRateBox->findData(port_conf.BaudRate) );
    ui->parityBox->setCurrentIndex( ui->parityBox->findData(port_conf.Parity) );
    ui->dataBitsBox->setCurrentIndex( ui->dataBitsBox->findData(port_conf.DataBits) );
    ui->stopBitsBox->setCurrentIndex( ui->stopBitsBox->findData(port_conf.StopBits) );
    ui->flowCtrlBox->setCurrentIndex( ui->flowCtrlBox->findData(port_conf.FlowControl) );
    ui->timeoutBox->setValue(port_conf.Timeout_Millisec);

}

void SerialSetupDialog::updateConfigToGui(PortSettings& port_conf)
{
    int idx;
    if ( (idx = ui->baudRateBox->currentIndex()) >=0 )
    {
        port_conf.BaudRate = (BaudRateType) ui->baudRateBox->itemData(idx).toInt();
    }
    if ( (idx = ui->parityBox->currentIndex()) >=0 )
    {
        port_conf.Parity = (ParityType) ui->parityBox->itemData(idx).toInt();
    }
    if ( (idx = ui->dataBitsBox->currentIndex()) >=0 )
    {
        port_conf.DataBits = (DataBitsType) ui->dataBitsBox->itemData(idx).toInt();
    }
    if ( (idx = ui->stopBitsBox->currentIndex()) >=0 )
    {
        port_conf.StopBits = (StopBitsType) ui->stopBitsBox->itemData(idx).toInt();
    }
    if ( (idx = ui->flowCtrlBox->currentIndex()) >=0 )
    {
        port_conf.FlowControl = (FlowType) ui->flowCtrlBox->itemData(idx).toInt();
    }
    port_conf.Timeout_Millisec = ui->timeoutBox->value();
}
