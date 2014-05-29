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
    ui->baudRateBox->addItem("1200",   QSerialPort::Baud1200);
    ui->baudRateBox->addItem("2400",   QSerialPort::Baud2400);
    ui->baudRateBox->addItem("4800",   QSerialPort::Baud4800);
    ui->baudRateBox->addItem("9600",   QSerialPort::Baud9600);
    ui->baudRateBox->addItem("19200",  QSerialPort::Baud19200);
    ui->baudRateBox->addItem("38400",  QSerialPort::Baud38400);
    ui->baudRateBox->addItem("57600",  QSerialPort::Baud57600);
    ui->baudRateBox->addItem("115200", QSerialPort::Baud115200);

    ui->baudRateBox->addItem("230400", 230400);
    ui->baudRateBox->addItem("460800", 460800);
    ui->baudRateBox->addItem("921600", 921600);

    ui->baudRateBox->setCurrentIndex(7);


    ui->parityBox->addItem("None",  QSerialPort::NoParity);
    ui->parityBox->addItem("Odd",   QSerialPort::OddParity);
    ui->parityBox->addItem("Even",  QSerialPort::EvenParity);
#if defined(Q_OS_WIN)
    ui->parityBox->addItem("Mark",  QSerialPort::MarkParity);
#endif
    ui->parityBox->addItem("Space", QSerialPort::SpaceParity);
    ui->parityBox->setCurrentIndex(0);


    ui->dataBitsBox->addItem("5", QSerialPort::Data5);
    ui->dataBitsBox->addItem("6", QSerialPort::Data6);
    ui->dataBitsBox->addItem("7", QSerialPort::Data7);
    ui->dataBitsBox->addItem("8", QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->stopBitsBox->addItem("1",    QSerialPort::OneStop);
#if defined(Q_OS_WIN)
    ui->stopBitsBox->addItem("1.5",  QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem("2",    QSerialPort::TwoStop);
    ui->parityBox->setCurrentIndex(0);


    ui->flowCtrlBox->addItem("None",       QSerialPort::NoFlowControl);
    ui->flowCtrlBox->addItem("Hardware",   QSerialPort::HardwareControl);
    ui->flowCtrlBox->addItem("Xon / Xoff", QSerialPort::SoftwareControl);
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
        port_conf.BaudRate = /*(QSerialPort::BaudRate)*/ ui->baudRateBox->itemData(idx).toInt();
    }
    if ( (idx = ui->parityBox->currentIndex()) >=0 )
    {
        port_conf.Parity = (QSerialPort::Parity) ui->parityBox->itemData(idx).toInt();
    }
    if ( (idx = ui->dataBitsBox->currentIndex()) >=0 )
    {
        port_conf.DataBits = (QSerialPort::DataBits) ui->dataBitsBox->itemData(idx).toInt();
    }
    if ( (idx = ui->stopBitsBox->currentIndex()) >=0 )
    {
        port_conf.StopBits = (QSerialPort::StopBits) ui->stopBitsBox->itemData(idx).toInt();
    }
    if ( (idx = ui->flowCtrlBox->currentIndex()) >=0 )
    {
        port_conf.FlowControl = (QSerialPort::FlowControl) ui->flowCtrlBox->itemData(idx).toInt();
    }
    port_conf.Timeout_Millisec = ui->timeoutBox->value();
}
