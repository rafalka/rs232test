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

#ifndef SERIALDEVICEINTERFACE_H
#define SERIALDEVICEINTERFACE_H

#include <QObject>
#include <QByteArray>

class SerialDeviceInterface : public QObject
{
    Q_OBJECT
public:
    explicit SerialDeviceInterface(QObject *parent = 0);
    virtual bool send(const QByteArray& buf)=0;
protected:
    virtual void show_driver_cfg_dlg() {}
signals:
    void on_receive(QByteArray& buf);
public slots:
    void do_config_dlg(){ show_driver_cfg_dlg(); }
    
};

#endif // SERIALDEVICEINTERFACE_H
