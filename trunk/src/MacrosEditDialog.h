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

#ifndef MACROSEDITDIALOG_H
#define MACROSEDITDIALOG_H

#include <QDialog>
#include <QVariantMap>

#include "BinaryEditor.h"


namespace Ui {
class MacrosEditDialog;
}

class MacrosEditDialog : public QDialog
{
    Q_OBJECT

protected:
    explicit MacrosEditDialog(QWidget *parent = 0);

    void             reportError(const QString message);
    void             setData(const QVariantMap& macros, bool replaceAll = true);
    QVariantMap      getData();

    bool             exportData( const QVariantMap& macros, QFile& file );
    bool             importData( QVariantMap& macros, QFile& file );

    void             setEditMode( BinaryEditor::input_modes_t edit_mode );
    QListWidgetItem* findMacroInList(const QString& name);
    void             updateAddBtn();
public:
    ~MacrosEditDialog();
    
    static bool Execute(QVariantMap& macros, BinaryEditor::input_modes_t edit_mode, QWidget *parent = 0)
    {
        MacrosEditDialog dlg(parent);

        dlg.setData( macros );
        dlg.setEditMode( edit_mode );
        bool result = dlg.exec() == QDialog::Accepted;

        if (result)
        {
            macros = dlg.getData();
        }
        return result;

    }

protected:
    void changeEvent(QEvent *e);
    
private slots:

    void on_macrosList_itemSelectionChanged();

    void on_addBtn_clicked();

    void on_macroNameEdit_textChanged(const QString &arg1);

    void on_delBtn_clicked();

    void on_macroBodyEdit_inputSizeChanged(int newsize);

    void on_exportBtn_clicked();

    void on_importBtn_clicked();

    void on_delAllBtn_clicked();

private:
    Ui::MacrosEditDialog *ui;
};

#endif // MACROSEDITDIALOG_H
