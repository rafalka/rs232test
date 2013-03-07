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

#ifndef SERIALSETUPDIALOG_H
#define SERIALSETUPDIALOG_H

#include <QDialog>
#include <qextserialport.h>

namespace Ui {
class SerialSetupDialog;
}

class SerialSetupDialog : public QDialog
{
    Q_OBJECT
    
protected:
    void setupUI();
    void updateGuiToConfig(PortSettings& port_conf);
    void updateConfigToGui(PortSettings& port_conf);
    explicit SerialSetupDialog(const QString portName, QWidget *parent = 0);
    ~SerialSetupDialog();
public:
    static bool Excecute(PortSettings& conf_ref, const QString portName, QWidget *parent = 0)
    {
        SerialSetupDialog dlg(portName,parent);
        dlg.updateGuiToConfig(conf_ref);
        bool result = dlg.exec() == QDialog::Accepted;

        if (result)
        {
            dlg.updateConfigToGui(conf_ref);
        }
        return result;
    }

private:
    Ui::SerialSetupDialog *ui;
};

#endif // SERIALSETUPDIALOG_H
