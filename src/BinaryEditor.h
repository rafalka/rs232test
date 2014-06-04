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

#ifndef BINARYEDITOR_H
#define BINARYEDITOR_H

#include <QStackedWidget>
#include <QPlainTextEdit>
#include "qhexedit.h"


class QStrBinConv;
class QBinStrConv;

// ******************************************************************************** C L A S S:  InputEditorAbstract
class InputEditorAbstract: public QObject
{
    Q_OBJECT
public:
    virtual QByteArray getInputData() = 0;
    virtual void       setInputData(QByteArray& data) = 0;
    virtual size_t     getInputSize() = 0;
    virtual int        getCurrentPos() = 0;
    virtual bool       getOverwriteMode() = 0;
    virtual QWidget*   getWidget() = 0;
    virtual void       undo() = 0;
    virtual void       redo() = 0;

public Q_SLOTS:
    void   refreshInput() {
        emit overwriteModeChanged( getOverwriteMode() );
        emit inputSizeChanged( getInputSize() );
        emit inputPosChanged( getCurrentPos() );
        emit inputChanged();
    }

signals:
    void   inputChanged();
    void   inputSizeChanged(int size);
    void   inputPosChanged(int pos);
    void   overwriteModeChanged(bool is_ovr_mode);
};

// ******************************************************************************** C L A S S:  InputTextEditor
class InputTextEditor: public InputEditorAbstract
{
    Q_OBJECT
protected:
    QPlainTextEdit* editor;
    QStrBinConv*    inputConv;
    QBinStrConv*    displayConv;
    size_t          last_doc_size;
public:
    InputTextEditor(QPlainTextEdit* widget);
    ~InputTextEditor();
    void setInputConv(QStrBinConv*  newConv);
    void setDisplayConv(QBinStrConv*  newConv);

    virtual QByteArray getInputData();
    virtual void       setInputData(QByteArray& data);
    virtual size_t     getInputSize();
    virtual int        getCurrentPos();
    virtual bool       getOverwriteMode();
    virtual QWidget*   getWidget()                     { return editor; }
    virtual void       undo()                          { editor->undo(); }
    virtual void       redo()                          { editor->redo(); }

protected Q_SLOTS:
    void on_editor_cursorPositionChanged();
    void on_editor_textChanged();

};

// ******************************************************************************** C L A S S:  InputHexEditor
class InputHexEditor: public InputEditorAbstract
{
    Q_OBJECT
protected:
    QHexEdit*       editor;
    size_t          last_doc_size;
public:
    InputHexEditor(QHexEdit* widget);
    ~InputHexEditor();

    virtual QByteArray getInputData()                 { return editor->data(); }
    virtual void       setInputData(QByteArray& data) { editor->setData(data); }
    virtual size_t     getInputSize()                 { return editor->data().size(); }
    virtual int        getCurrentPos()                { return editor->cursorPosition(); }
    virtual bool       getOverwriteMode()             { return editor->overwriteMode(); }
    virtual QWidget*   getWidget()                    { return editor; }
    virtual void       undo()                         { editor->undo(); }
    virtual void       redo()                         { editor->redo(); }

public Q_SLOTS:
    void on_editor_currentAddressChanged (int address);
    void on_editor_currentSizeChanged (int size);
    void on_editor_dataChanged ();
    void on_editor_overwriteModeChanged (bool state);

};


namespace Ui {
class BinaryEditor;
}

class BinaryEditor : public QStackedWidget
{
    Q_OBJECT
    
protected:
    void changeEvent(QEvent *e);

    InputEditorAbstract* current_editor;
    QByteArray           _data_cache;
    bool                 _data_cache_valid;

    void                 updateDataCache(const QByteArray& data) { _data_cache = data;  _data_cache_valid=true; }
    void                 updateDataCache() { if (!_data_cache_valid) {updateDataCache(current_editor->getInputData()); } }
public:
    typedef enum {
        INMODE_TEXT,
        INMODE_HEX,

        __INMODES_CNT
    } input_modes_t;

    void       setEditMode(input_modes_t mode, bool forceRefresh = false );
    void       setTextModeCoverters(QStrBinConv* inputConverter, QBinStrConv*  displayConverter);

    QByteArray getInputData()
    {
        updateDataCache();
        return _data_cache;
    }

    const QByteArray& getInputConstData()
    {
        updateDataCache();
        return _data_cache;
    }

    void       setInputData( QByteArray& data)
    {
        current_editor->setInputData(data);
        updateDataCache(data);

        refreshInput();
    }
    size_t     getInputSize()
    {
        updateDataCache();
        return _data_cache.size();
    }
    int        getCurrentPos()                { return current_editor->getCurrentPos(); }
    bool       getOverwriteMode()             { return current_editor->getOverwriteMode(); }

    explicit BinaryEditor(QWidget *parent = 0);
    ~BinaryEditor();
    

private:
    Ui::BinaryEditor *ui;

    InputHexEditor*   hex_editor;
    InputTextEditor*  txt_editor;

    void connectInputEditor(InputEditorAbstract *editor);

public slots:

    void   refreshInput() {
        emit overwriteModeChanged( getOverwriteMode() );
        emit inputSizeChanged( getInputSize() );
        emit inputPosChanged( getCurrentPos() );
        emit inputChanged();
    }
    void clear();
    void undo();
    void redo();


protected Q_SLOTS:
    void onEditorCurrentAddressChanged (int address);
    void onEditorCurrentSizeChanged (int size);
    void onEditorDataChanged ();
    void onEditorOverwriteModeChanged (bool state);

signals:
    void   inputChanged();
    void   inputSizeChanged(int size);
    void   inputPosChanged(int pos);
    void   overwriteModeChanged(bool is_ovr_mode);

};

#endif // BINARYEDITOR_H
