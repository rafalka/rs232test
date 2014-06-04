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

#include "BinaryEditor.h"
#include "ui_BinaryEditor.h"

#include "strbinconv.h"

#include "debug.h"


// ******************************************************************************** C L A S S: BinaryEditor

BinaryEditor::BinaryEditor(QWidget *parent) :
    QStackedWidget(parent),
    current_editor(NULL),
    _data_cache_valid(false),
    ui(new Ui::BinaryEditor)
{
    ui->setupUi(this);

    ui->inputHexEdit->setOverwriteMode(false);
    ui->inputHexEdit->setFont( ui->inputTextEdit->font() );

    hex_editor = new InputHexEditor(ui->inputHexEdit);
    txt_editor = new InputTextEditor(ui->inputTextEdit);

    txt_editor->setInputConv(QStrBinConvCollection::getConv(QStrBinConvCollection::CONV_ASCII) );
    txt_editor->setDisplayConv(QBinStrConvCollection::getConv(QBinStrConvCollection::CONV_ASCII) );

    connectInputEditor(hex_editor);
    connectInputEditor(txt_editor);

    setEditMode(INMODE_TEXT);

}

BinaryEditor::~BinaryEditor()
{
    delete ui;
}

void BinaryEditor::changeEvent(QEvent *e)
{
    QStackedWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void BinaryEditor::connectInputEditor(InputEditorAbstract *editor)
{
    ASSERT_ALWAYS( connect(editor, SIGNAL( inputPosChanged(int) ),       SLOT( onEditorCurrentAddressChanged(int) ) ) );
    ASSERT_ALWAYS( connect(editor, SIGNAL( overwriteModeChanged(bool) ), SLOT( onEditorOverwriteModeChanged(bool) ) ) );
    ASSERT_ALWAYS( connect(editor, SIGNAL( inputSizeChanged(int) ),      SLOT( onEditorCurrentSizeChanged(int) )    ) );
    ASSERT_ALWAYS( connect(editor, SIGNAL( inputChanged() ),             SLOT( onEditorDataChanged() )              ) );
}

void BinaryEditor::clear()
{
    QByteArray empty;
    setInputData(empty);
}

void BinaryEditor::undo()
{
    current_editor->undo();
}

void BinaryEditor::redo()
{
    current_editor->redo();
}

void BinaryEditor::setEditMode(BinaryEditor::input_modes_t mode, bool forceRefresh )
{
    InputEditorAbstract* new_editor=NULL;

    switch (mode)
    {
    case INMODE_HEX:
        setCurrentWidget(ui->hexInPage);
        new_editor = hex_editor;
        break;
    case INMODE_TEXT:
    default:
        setCurrentWidget(ui->txtInPage);
        new_editor = txt_editor;
        mode = INMODE_TEXT;
    }

    if ( (current_editor!=new_editor) || forceRefresh )
    {
        if (current_editor) updateDataCache(current_editor->getInputData());
        current_editor=new_editor;

        if (_data_cache.size()>0)
            current_editor->setInputData(_data_cache);
        else
            _data_cache_valid = false;
    }
}

void BinaryEditor::setTextModeCoverters(QStrBinConv *inputConverter, QBinStrConv *displayConverter)
{
    if ( current_editor==txt_editor )
    {
        updateDataCache( current_editor->getInputData() );
    }

    txt_editor->setInputConv( inputConverter );
    txt_editor->setDisplayConv( displayConverter );

    if ( current_editor==txt_editor )
    {
        current_editor->setInputData(_data_cache);
        _data_cache_valid = false;
    }
}


void BinaryEditor::onEditorCurrentAddressChanged(int address)
{
    emit inputPosChanged(address);
}

void BinaryEditor::onEditorCurrentSizeChanged(int size)
{
    _data_cache_valid = false;
    emit inputSizeChanged(size);
}

void BinaryEditor::onEditorDataChanged()
{
    _data_cache_valid = false;
    emit inputChanged();
}

void BinaryEditor::onEditorOverwriteModeChanged(bool state)
{
    emit overwriteModeChanged(state);
}

// ******************************************************************************** C L A S S: InputTextEditor

InputTextEditor::InputTextEditor(QPlainTextEdit *widget)
    : editor(widget)
    , inputConv(NULL)
    , last_doc_size(0)
{

    //QMetaObject::connectSlotsByName(this);
    ASSERT_ALWAYS( connect(editor, SIGNAL(textChanged()),           SLOT(on_editor_textChanged()) ) );
    ASSERT_ALWAYS( connect(editor, SIGNAL(cursorPositionChanged()), SLOT(on_editor_cursorPositionChanged()) ) );

}


InputTextEditor::~InputTextEditor()
{

}

void InputTextEditor::setInputConv(QStrBinConv*  newConv)
{
    QStrBinConv*  conv = inputConv;
    inputConv = NULL;
    if (conv) QStrBinConvCollection::disposeConv(conv);
    inputConv = newConv;
}

void InputTextEditor::setDisplayConv(QBinStrConv *newConv)
{
    QBinStrConv*  conv = displayConv;
    displayConv = NULL;
    if (conv) QBinStrConvCollection::disposeConv(conv);
    displayConv = newConv;
}


QByteArray InputTextEditor::getInputData()
{
    QByteArray buf;

    if ( inputConv )
    {
        QString    str = editor->toPlainText();
        int        erridx;

        switch (inputConv->convert(str,&buf,&erridx))
        {
            case QStrBinConv::VALID:
                {
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

    return buf;
}

void InputTextEditor::setInputData(QByteArray &data)
{
    if (displayConv)
    {
        editor->setPlainText( displayConv->convert(data) );
    }
}

size_t InputTextEditor::getInputSize()
{
    int size = editor->document()->characterCount();
    return (size>0) ? (size_t) size : 0;
}

int InputTextEditor::getCurrentPos()
{
    return editor->textCursor().position();
}

bool InputTextEditor::getOverwriteMode()
{
    return editor->overwriteMode ();
}

void InputTextEditor::on_editor_cursorPositionChanged()
{
    emit inputPosChanged( getCurrentPos() );
}

void InputTextEditor::on_editor_textChanged()
{
    size_t new_size = getInputSize();
    if (new_size!=last_doc_size)
    {
        last_doc_size=new_size;
        emit inputSizeChanged(new_size);
    }

    emit inputChanged();
}

// ******************************************************************************** C L A S S: InputHexEditor


InputHexEditor::InputHexEditor(QHexEdit *widget)
    : editor(widget)
{
    ASSERT_ALWAYS( connect(editor, SIGNAL( currentAddressChanged(int) ), SLOT( on_editor_currentAddressChanged(int) ) ) );
    ASSERT_ALWAYS( connect(editor, SIGNAL( overwriteModeChanged(bool) ), SLOT( on_editor_overwriteModeChanged(bool) ) ) );
    ASSERT_ALWAYS( connect(editor, SIGNAL( currentSizeChanged(int) ),    SLOT( on_editor_currentSizeChanged(int) )    ) );
    ASSERT_ALWAYS( connect(editor, SIGNAL( dataChanged() ),              SLOT( on_editor_dataChanged() )              ) );
}

InputHexEditor::~InputHexEditor()
{
}

void InputHexEditor::on_editor_currentAddressChanged(int address)
{
    emit inputPosChanged(address);
}

void InputHexEditor::on_editor_currentSizeChanged(int size)
{
    emit inputSizeChanged(size);
}

void InputHexEditor::on_editor_dataChanged()
{
    emit inputChanged();
}

void InputHexEditor::on_editor_overwriteModeChanged(bool state)
{
    emit overwriteModeChanged(state);
}
