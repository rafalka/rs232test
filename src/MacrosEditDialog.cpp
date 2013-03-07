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

#include <QFileDialog>
#include <QDataStream>
#include <QMessageBox>

#include "MacrosEditDialog.h"
#include "ui_MacrosEditDialog.h"

//static const uint8_t macros_file_magic[16] = "RS232TOOLMACROS";
static const quint32 macros_file_magic = 0x52544d43;

#define MACRO_FILE_EXT "*.mac"
//static const char* macro_file_dialog_default_filter = MACRO_FILE_EXT;
static const char* macro_file_dialog_filters =
        "Macro Files (" MACRO_FILE_EXT ");;All files (*.*)";

MacrosEditDialog::MacrosEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MacrosEditDialog)
{
    ui->setupUi(this);

    ui->verticalLayout_2->setContentsMargins(4, 4, 4, 4);
    ui->horizontalLayout_2->setContentsMargins(4, 4, 4, 4);
    ui->splitter->setStretchFactor ( 0, 1 ); // List Part
    ui->splitter->setStretchFactor ( 1, 1 ); // Edit part

}

void MacrosEditDialog::reportError(const QString message)
{
    QMessageBox::warning(this,
                         tr("Warning!"),
                         message);
}

void MacrosEditDialog::setData(const QVariantMap &macros, bool replaceAll)
{

    if (replaceAll)
    {
        ui->macrosList->clear();
    }

    QVariantMap::const_iterator i = macros.constBegin();
    QListWidgetItem*            item;

    while (i != macros.constEnd())
    {
        const QVariant& n = i.value();
        if (  n.canConvert(QVariant::ByteArray) )
        {
            item = new QListWidgetItem(i.key(),ui->macrosList );
            item->setData(Qt::UserRole,n);
        }
        ++i;
    }
     ui->macrosList->clearSelection();
}

QVariantMap MacrosEditDialog::getData()
{
    QVariantMap                 macros;
    QListWidgetItem*            item;
    int                         cnt;

    for (cnt=0;cnt<ui->macrosList->count();cnt++)
    {
        item = ui->macrosList->item(cnt);
        if (item)
        {
            macros[item->text()] = item->data(Qt::UserRole);
        }
    }

    return macros;
}

bool MacrosEditDialog::exportData(const QVariantMap &macros, QFile &file)
{
    QDataStream out(&file);

    // Write a header with a "magic number" and a version
    out.setVersion(QDataStream::Qt_4_0);
    out << macros_file_magic;
    //out << macros;

    QVariantMap::const_iterator i = macros.constBegin();

    while (i != macros.constEnd())
    {

        out << i.key();
        out << i.value().toByteArray();

        ++i;
    }



    return true;
}

bool MacrosEditDialog::importData(QVariantMap &macros, QFile &file)
{
    QDataStream in(&file);

    // Read and check the header
    quint32 magic;
    in >> magic;
    if (magic!=macros_file_magic) return false;
//    in >> macros;
    QString     name;
    QByteArray  value;

    macros.clear();
    while (! in.atEnd() )
    {
        in >> name;
        in >> value;
        macros[name] = value;
    }

    return true;
}

void MacrosEditDialog::setEditMode(BinaryEditor::input_modes_t edit_mode)
{
    ui->macroBodyEdit->setEditMode(edit_mode);
}

QListWidgetItem *MacrosEditDialog::findMacroInList(const QString &name)
{
    QList<QListWidgetItem *> items = ui->macrosList->findItems(name,Qt::MatchExactly);

    return items.isEmpty() ? NULL : items.first();
}

void MacrosEditDialog::updateAddBtn()
{
    ui->delBtn->setEnabled( ui->macrosList->currentItem()!=NULL );

    const QString arg1 = ui->macroNameEdit->text();

    if (! arg1.isEmpty())
    {
        ui->addBtn->setText( findMacroInList(arg1)!=NULL ? tr("Replace") : tr("Add") );
    }
    ui->addBtn->setEnabled( (! arg1.isEmpty() ) && ui->macroBodyEdit->getInputSize()>0 );
}

MacrosEditDialog::~MacrosEditDialog()
{
    delete ui;
}

void MacrosEditDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void MacrosEditDialog::on_macrosList_itemSelectionChanged()
{
    QListWidgetItem *current = ui->macrosList->currentItem();

    if (current)
    {
        QByteArray  arr = current->data(Qt::UserRole).toByteArray();
        ui->macroNameEdit->setText( current->text() );
        ui->macroBodyEdit->setInputData(arr);
    }


    ui->delBtn->setEnabled( (current&&current->isSelected()) );
}


void MacrosEditDialog::on_addBtn_clicked()
{
    const QString               name = ui->macroNameEdit->text();
    if (name.isEmpty()) return;

    QListWidgetItem*            item = findMacroInList(name);

    if (!item)
    {
        item = new QListWidgetItem(name,ui->macrosList );
    }
    item->setData(Qt::UserRole,ui->macroBodyEdit->getInputData());

     updateAddBtn();
}

void MacrosEditDialog::on_macroNameEdit_textChanged(const QString &arg1)
{
    (void)arg1;
    updateAddBtn();
}

void MacrosEditDialog::on_delBtn_clicked()
{
    QListWidgetItem *current = ui->macrosList->currentItem();

    if (current)
    {
        if ( QMessageBox::Yes ==
                QMessageBox::question(
                    this,
                    tr("Deleting macro..."),
                    QString(tr("You are about to delete %1\nAre you sure?").arg(current->text()) ),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                )
            )
        {
            delete current;
            updateAddBtn();
        }
    }

}
void MacrosEditDialog::on_delAllBtn_clicked()
{
    if ( QMessageBox::Yes ==
            QMessageBox::question(
                this,
                tr("Deleting all macros..."),
                tr("Are you sure to delete ALL macros?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            )
        )
    {
        ui->macrosList->clear();
        updateAddBtn();
    }

}

void MacrosEditDialog::on_macroBodyEdit_inputSizeChanged(int newsize)
{
    (void) newsize;
    updateAddBtn();
}

void MacrosEditDialog::on_exportBtn_clicked()
{
#ifdef OP_NAME
# undef OP_NAME
#endif
#define OP_NAME "Macros export:"
    QVariantMap data = getData();

    if (data.size()<=0)
    {
        reportError(OP_NAME "Cannot export. List is empty");
        return;
    }

    QString file_name = QFileDialog::getSaveFileName(this,
                             tr("Export macros"),
                             QString(), tr(macro_file_dialog_filters)
                          );
    if ( file_name.isEmpty() ) return;

    QFile  file(file_name);
    if (! file.open(QIODevice::WriteOnly) )
    {
        reportError(QString(OP_NAME "Cannot open for writing file: %1").arg(file_name) );
        return;
    }

    if (! exportData(data, file) )
    {
        reportError(OP_NAME "Export operation failed!");
    }

}

void MacrosEditDialog::on_importBtn_clicked()
{
#ifdef OP_NAME
# undef OP_NAME
#endif
#define OP_NAME "Macros import:"


    QString file_name = QFileDialog::getOpenFileName(this,
                             tr("Import macros"),
                             QString(), tr(macro_file_dialog_filters)
                          );
    if ( file_name.isEmpty() ) return;

    QFile  file(file_name);
    if (! file.open(QIODevice::ReadOnly) )
    {
        reportError(QString(OP_NAME "Cannot open for reading file: %1").arg(file_name) );
        return;
    }

    QVariantMap data;

    if (! importData(data, file) )
    {
        reportError(OP_NAME "Import operation failed!");
        return;
    }

    setData(data,false);

}


