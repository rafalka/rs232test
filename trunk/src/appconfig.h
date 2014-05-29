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

#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QSettings>
#include <QVariant>
#include <QString>
#include <QComboBox>
#include <QVariantList>
#include <QVariantMap>

#include "parseopt.h"

extern QSettings* appconfig;

typedef enum
{
    CONF_OP_READ,
    CONF_OP_WRITE
} cfg_operations_t;

template <typename T, class conv> class AutoCfg
{
protected:
    AutoCfg(){;}
    T  *        _cfgvar;
    const char* _cfgname;
public:
    static bool readCfg(T* cfgvar, const char* cfgname)
    {
        QVariant var    = appconfig->value(cfgname);
        bool     result = !var.isNull();
        if (result)
        {
             conv::fromVar(var,cfgvar);
        }

        return result;
    }
    static bool writeCfg(T* cfgvar, const char* cfgname)
    {
        QVariant var;
        conv::toVar(cfgvar,var);
        bool     result = !var.isNull();
        if (result)
        {
            appconfig->setValue(cfgname, var );
        }

        return result;
    }
    static bool doCfg(cfg_operations_t operation, T* cfgvar, const char* cfgname )
    {
        switch (operation)
        {
        case CONF_OP_READ:  return readCfg (cfgvar,cfgname);
        case CONF_OP_WRITE: return writeCfg(cfgvar,cfgname);
        }
        return false;
    }

    AutoCfg(T* cfgvar, const char* cfgname)
        :_cfgvar(cfgvar)
        ,_cfgname(cfgname)
    {
        readCfg(cfgvar,cfgname);
    }
    ~AutoCfg()
    {
       writeCfg(_cfgvar,_cfgname);
    }

};


template <typename T> class convVarAsIntTo
{
public:
    static void fromVar(QVariant& var, T* val) { *val = static_cast<T>(var.toInt()); }
    static void toVar(T* val, QVariant& var )  {  var = static_cast<int>(*val); }
};

/*
class convVarTo_int
{
public:
    static void fromVar(QVariant& var, int* val) { *val =  var.toInt(); }
    static void toVar(int* val, QVariant& var )  {  var = *val; }
};
*/
typedef convVarAsIntTo<int> convVarTo_int;

class convVarTo_QString
{
public:
    static void fromVar(QVariant& var, QString* val) { *val = var.toString(); }
    static void toVar(QString* val, QVariant& var )  {  var = *val; }
};


class convVarTo_QVariantList
{
public:
    static void fromVar(QVariant& var, QVariantList* val) { *val = var.toList(); }
    static void toVar(QVariantList* val, QVariant& var )  {  var = *val; }
};

class convVarTo_QVariantMap
{
public:
    static void fromVar(QVariant& var, QVariantMap* val) { *val = var.toMap(); }
    static void toVar(QVariantMap* val, QVariant& var )  {  var = *val; }
};

class convVarTo_QComboBox
{
public:
    static void fromVar(QVariant& var, QComboBox* val)
    {
        val->addItems( var.toStringList() );
    }
    static void toVar(QComboBox* val, QVariant& var )
    {
        QStringList items;
        int         itemscnt = val->count();

        items.reserve(itemscnt);

        for (int cnt = 0; cnt<itemscnt; cnt++)
        {
            items.append(val->itemText(cnt));
        }
        var = items;
    }
};

typedef AutoCfg< int,          convVarTo_int          >  AutoCfg_int;
typedef AutoCfg< QString,      convVarTo_QString      >  AutoCfg_QString;
typedef AutoCfg< QComboBox,    convVarTo_QComboBox    >  AutoCfg_QComboBox;
typedef AutoCfg< QVariantList, convVarTo_QVariantList >  AutoCfg_QVariantList;
typedef AutoCfg< QVariantMap,  convVarTo_QVariantMap  >  AutoCfg_QVariantMap;


#define AUTOCFG_ITEM(_type_,_name_) AutoCfg_##_type_ autocfg_##_name_

#if 0
class AutoCfgInt
{
protected:
    AutoCfgInt(){;}
    int*        _cfgvar;
    const char* _cfgname;
public:
    AutoCfgInt(int* cfgvar, const char* cfgname)
        :_cfgvar(cfgvar)
        ,_cfgname(cfgname)
    {
        QVariant var=appconfig->value(_cfgname);
        if (!var.isNull())
        {
            *cfgvar = var.toInt();
        }
    }
    ~AutoCfgInt()
    {
       appconfig->setValue(_cfgname,*_cfgvar);
    }
};
#endif

#define CONF_MARK_USE_DEFAULT_STR  ((char*) 1)

class AppCommandLineParams
{
protected:
    opt_defs_t          options[8];// NOTE: thihs must be always equal to number of parameters +1
    static opt_val_listitem_t   inmode_list[];
    static opt_val_listitem_t   outmode_list[];
public:
    char*       selPort;
    char*       configFileName;
    char*       logFileName;
    int         displayMode;
    int         editMode;
    char        helpRequeted;
    char        doConnect;
    //NOTE: If you add parameter here, dont forget increasing options[]
    //      ... and updating constructor...

    AppCommandLineParams();
    bool parseCommandLine(int argc, char *argv[]);
    QString getCommandLineHelpString();
};

extern AppCommandLineParams appcmdline;

#endif // APPCONFIG_H
