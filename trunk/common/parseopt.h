/***************************************************************************
 * @file     parseopt.h                                                     
 * @brief    High level command parsing API declaration
 *
 * @author   Rafal Kukla < rkdevel@gmail.com >
 *
 ***************************************************************************
 *               Copyright (C) 2010 Rafal Kukla         
 *                                                                         
 *  This file is a part of libcsupp library and is distributed under       
 *  two licenses (full license text is available in file LICESE):          
 *  - For the use inside Motorola Inc.:                                    
 *    The libcsupp library may be dynamically or statically linked         
 *    with any Motorola product without ANY restrictions.                  
 *    Full license text is available in file LICENSE.MOT                   
 *                                                                         
 *  - For any other usage, it is available under GPLv2:                    
 *    Full license text is available in file LICENSE.GPL2                  
 ***************************************************************************/



//#error Nie skoñczone to jeszcze

/** TODO: Implementacja named parameters
Ide¹ named parameters jest, ¿e mo¿na podawaæ parametry pozycyjne przez nazwê arg_name
<NIE!: nazwany parametr pozycyjny rozpoznawany jest przez short_opt==-1>

To mocno komplikuje obs³ugê parametrów pozycyjnych, a w szczególnoœæi ich tablic
Usuniêty zostanie pseudo parametr tablicowy. Teraz ka¿dy parametr bêdzie móg³ byæ 
tablic¹. Umo¿liwiaj¹ to wprowadzone pola occ_max, elem_size, occurences.
A wszystko to potrzebne, by zaimplementowaæ embedded commandline do definiowania substruktur

IDEA:
parametry oddzielone przecinkami - dla listy mogly by stanowic ORowane wartosci (dla opcji)
w innych zastosowaniach moglyby tworzyc tablice.
Q: czy zapis --opcja=ala,ma,kota powinien byc rownoznaczny do --opcja=ala --opcja=ma --opcja=kota ???

*/



#ifndef _PARSEOPT_H
#define _PARSEOPT_H 1

#include <stdarg.h>

#ifdef __GNU_LIBRARY__
#include <getopt.h>
#else
//TODO: remove #include "getopt.h"
//#include "getopt.h"
#endif



#define MAX_ALIASES 8 /* Max num of aliases for list type of parameter */

//TODO: Move it back to parsept.c
#define OPT_BOOL_TRUE_VALS  "True","Yes","Y","1"
#define OPT_BOOL_FALSE_VALS "False","No","N","0"
#define OPT_BOOL_VALS "True/False, Yes/No,Y/N,1/0"


#ifdef	__cplusplus
extern "C" {
#endif

typedef struct opt_defs opt_defs_t;

typedef enum {
  otNull,
  otChar,
  otString, /*Przeformatowany ³añcuch mog¹cy zawieraæ konstrukcjê typu \n \t itd*/
  otInt,
  otFloat,
  otBool,
  otCallback,
  otList,   /* parameter should be a name defined in list passed as opt_min.l
            List of names supports aliases, separated by | */
  otSubcmnd   /* parameter is a sub commandline, ie "addr=1; size=1; run=FALSE" */
} opt_val_types;

typedef enum {
  PO_NO_ARG=0,
  PO_REQUIRED_ARG=1,
  PO_REQ_ARG=PO_REQUIRED_ARG,
  PO_OPTIONAL_ARG=2,
  PO_OPT_ARG=PO_OPTIONAL_ARG
} opt_arg_req;

typedef struct opt_val_listitem
{
  int           Id;     ///< List item value
  const char*   Name;   ///< List item name
  const char*   Desc;   ///< Description
  int           Mask;   ///< Value mask
} opt_val_listitem_t;

#define PUT_LIST_ENTRY(olp, _item_value_ , _item_name_ , _item_desc_ , _val_update_mask_ ) \
{ \
    olp->Id   = (_item_value_); \
    olp->Name = (_item_name_); \
    olp->Desc = (_item_desc_); \
    olp->Mask = (_val_update_mask_); \
    olp++; \
}


typedef union 
{
  int    i;
  char*  c;
  double f;
  char   b;
  void*  v;
  opt_val_listitem_t* l;
} opt_val;

typedef union 
{
  void*   v;
  char**  c;
  int*    i;
  double* f;
  char*   b;
  int     (*cb) (const opt_defs_t *opt, char* optarg);
} opt_val_ptr;

enum ELEM_OPERATION
{
  ELOP_SET  =   0, ///< Element value will be replaced by new value
  ELOP_OR   =  -1, ///< Element value will be ORed with new value
  ELOP_AND  =  -2, ///< Element value will be ANDed with new value
  ELOP_XOR  =  -3, ///< Element value will be XORed with new value
  ELOP_NAND =  -4, ///< Element value will be ANDed with negation of new value
  ELOP_ADD  =  -5, ///< New value will be added to element value 
  ELOP_SUB  =  -6, ///< New value will be substacted from element value 
  ELOP_MUL  =  -7, ///< Element value will be multipled by new value
  ELOP_DIV  =  -9,  ///< Element value will be divided by new value


  ELOP_LAST_VALID = -10
};
#define IS_ELOP_VALID(_op_) ( (_op_) > ELOP_LAST_VALID )
#define IS_ELOP_SET(_op_)   ( (_op_) > ELOP_OR )
#define IS_ELOP_LOG(_op_)   ( IS_ELOP_SET(_op_) || ( ((_op_) <= ELOP_OR)  && ((_op_) >= ELOP_NAND) ) )
#define IS_ELOP_ARTM(_op_)  ( IS_ELOP_SET(_op_) || ( ((_op_) <= ELOP_ADD) && ((_op_) >= ELOP_DIV) ) )

#define ELOP(_op_,_el_,_val_)                        \
{                                                    \
  if ( (_op_) >= ELOP_SET )                          \
  {                                                  \
    _el_ = (_val_);                                  \
  }                                                  \
  else switch(_op_)                                  \
  {                                                  \
    case ELOP_OR:   _el_ |= (_val_); break;          \
    case ELOP_AND:  _el_ &= (_val_); break;          \
    case ELOP_XOR:  _el_ ^= (_val_); break;          \
    case ELOP_NAND: _el_ &= ~(_val_); break;         \
    case ELOP_ADD:  _el_ += (_val_); break;          \
    case ELOP_SUB:  _el_ -= (_val_); break;          \
    case ELOP_MUL:  _el_ *= (_val_); break;          \
    case ELOP_DIV:  _el_ /= (_val_); break;          \
  }                                                  \
}
#define ELOPSET(_op_,_el_,_val_)                     \
{                                                    \
  if ( (_op_) >= ELOP_SET )                          \
  {                                                  \
    _el_ = (_val_);                                  \
  }                                                  \
}
#define ELOPLOG(_op_,_el_,_val_,_on_error_)          \
{                                                    \
  if ( (_op_) >= ELOP_SET )                          \
  {                                                  \
    _el_ = (_val_);                                  \
  }                                                  \
  else switch(_op_)                                  \
  {                                                  \
    case ELOP_OR:   _el_ |= (_val_); break;          \
    case ELOP_AND:  _el_ &= (_val_); break;          \
    case ELOP_XOR:  _el_ ^= (_val_); break;          \
    case ELOP_NAND: _el_ &= ~(_val_); break;         \
  }                                                  \
}
#define ELOPARTM(_op_,_el_,_val_)                    \
{                                                    \
  if ( (_op_) >= ELOP_SET )                          \
  {                                                  \
    _el_ = (_val_);                                  \
  }                                                  \
  else switch(_op_)                                  \
  {                                                  \
    case ELOP_ADD:  _el_ += (_val_); break;          \
    case ELOP_SUB:  _el_ -= (_val_); break;          \
    case ELOP_MUL:  _el_ *= (_val_); break;          \
    case ELOP_DIV:  _el_ /= (_val_); break;          \
  }                                                  \
}

/************************************ STRUCT OPT_DEF *****************************
 Struktura steruj¹ca dla funkcji parse_opt(). Pola:
 short_opt     - Znak krótkiej opcji -c lub #0 jeœli brak,
                 Jeœli -1, to jest to nazwany parametr pozycyjny
 long_opt      - Nazwa d³ugiej opcji --nazwa lub NULL jeœli brak
 has_arg       - pole kompatybilne z polem struktury option dla funkcji getopt_long()
                 Okreœla, czy opcja ma argument. Mo¿liwe wartoœci:
                 PO_NO_ARG (0) - opcja nie ma argumentu
                 PO_REQUIRED_ARG (1) - opcja posiada argument obowi¹zkowy
                 PO_OPTIONAL_ARG (2) - opcja posiada argument opcjonalny
 arg_name      - nazwa argumentu
 val_type      - Typ danych opcji/argumentu. Mo¿liwe wartoœci
                  otChar=1 - ³añcuch znaków
                  otString - przeformatowany ³añcuch mog¹cy zawieraæ konstrukcjê typu \n \t itd
                  otInt    - wartoœæ integer
                  otFloat  - wartoœæ double float
                  otBool   - wartoœæ char, 1 - jeœli argument jest jednym z napisów:
                             "True","Yes","T","Y","1", w przeciwnym razie 0
                  otCallback - po napotkaniu tego argumentu bedzie wywo³ana funkcja u¿ytkownika
                             z parametrami: (const struct opt_def *opt, char* optarg), gdzie
                             opt_def - wskaŸnik do tej struktury,
                             optarg  - argument tej opcji
 val           - wska¿nik do zmiennej w której zostanie umieszczona wartoœæ argumentu opcji, lub
                 wska¿nik do funkcji u¿ytkownika jeœli val_type==otCallback
 opt_def       - domyœlna wartoœæ argumentu opcji, u¿ywana jeœli argumentu nie zdefiniowano lub
                 jeœli has_arg==PO_NO_ARG
 opt_min,opt_max - minimalna i maksymalna wartoœæ argumentu w przypadku, gty typ danych 
                 opcji/argumentu to integer lub float. Jeœli opt_min>opt_max, to sprawdzenie
                 zakresu nie jest wykonywane.
 desc          - Opcjonalny opis opcji. Nie u¿ywany przez parse_opt().

 Jeœli short_opt=0 i long_opt=NULL, to struktura okresla parametr pozycyjny
**********************************************************************************/

struct opt_defs {
  char          short_opt;       ///< Short option char
  const char*   long_opt;        ///< Long option name
  opt_arg_req   has_arg;         ///< Option parametr indicator
  const char*   arg_name;        ///< Argument name
  opt_val_types val_type;        ///< Option parametr type
  opt_val_ptr   val;             ///< Option parametr value pointer
  opt_val       opt_def;         ///< default option parametr if has_arg is PO_OPTIONAL_ARG or PO_NO_ARG
  opt_val       opt_min;         ///< low option parametr range
  opt_val       opt_max;         ///< high option parametr range
  const char*   desc;            ///< Option description
  unsigned      occ_max;         ///< Maximum allowed occurences(array size)
  int           elem_size;       ///< sizeof element for array definitions, if 0, then each next occurence will gonna modify same element
                                 ///< Negative values have special meaning, defined in ELEM_OPERATION
  unsigned      occurences;      ///< This field is modified by parse_opt() and updated to count of occurences 
};


// definicje makr do odk³adania parametrów do tablicy konfiguracji
// U¿ywane ze wzgledu na beznadziejn¹ formê inicjalizacji uni w C
#define PUT_OPT_DEF_BOOL(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,pcVal,cValTrue,cValFalse,cOptMax,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otBool;     \
  pOptDef->val.b=pcVal;           pOptDef->opt_def.b=cValTrue;  \
  pOptDef->opt_min.b=cValFalse;   pOptDef->opt_max.b=cOptMax;   \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_BOOL_OPT(pOptDef,pShortOpt,pcLongOpt,pcVal,pcDesc) \
  PUT_OPT_DEF_BOOL(pOptDef,pShortOpt,pcLongOpt,PO_NO_ARG,NULL,pcVal,1,0,1,1,0,pcDesc)

#define PUT_OPT_DEF_CHAR(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,ppcVal,pcOptDef,pcOptMin,pcOptMax,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otChar;     \
  pOptDef->val.c=ppcVal;          pOptDef->opt_def.c=pcOptDef;  \
  pOptDef->opt_min.c=pcOptMin;    pOptDef->opt_max.c=pcOptMax;  \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_CHAR_OPT(pOptDef,pShortOpt,pcLongOpt,pcArgName,ppcVal,pcOptDef,pcDesc) \
  PUT_OPT_DEF_CHAR(pOptDef,pShortOpt,pcLongOpt,PO_REQUIRED_ARG,pcArgName,ppcVal,pcOptDef,NULL,NULL,1,0,pcDesc)

#define PUT_OPT_DEF_STRING(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,ppcVal,pcOptDef,pcOptMin,pcOptMax,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otString;   \
  pOptDef->val.c=ppcVal;          pOptDef->opt_def.c=pcOptDef;  \
  pOptDef->opt_min.c=pcOptMin;    pOptDef->opt_max.c=pcOptMax;  \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_STRING_OPT(pOptDef,pShortOpt,pcLongOpt,pcArgName,ppcVal,pcOptDef,iMaxOcc,iItemSize,pcDesc) \
  PUT_OPT_DEF_STRING(pOptDef,pShortOpt,pcLongOpt,PO_REQUIRED_ARG,pcArgName,ppcVal,pcOptDef,NULL,NULL,1,0,pcDesc)

#define PUT_OPT_DEF_INT(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,piVal,iOptDef,iOptMin,iOptMax,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otInt;      \
  pOptDef->val.i=piVal;           pOptDef->opt_def.i=iOptDef;   \
  pOptDef->opt_min.i=iOptMin;     pOptDef->opt_max.i=iOptMax;   \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_INT_OPT(pOptDef,pShortOpt,pcLongOpt,pcArgName,piVal,iOptDef,iOptMin,iOptMax,pcDesc) \
  PUT_OPT_DEF_INT(pOptDef,pShortOpt,pcLongOpt,PO_REQUIRED_ARG,pcArgName,piVal,iOptDef,iOptMin,iOptMax,1,0,pcDesc)

#define PUT_OPT_DEF_FLOAT(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,pdVal,dOptDef,dOptMin,dOptMax,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otFloat;    \
  pOptDef->val.f=pdVal;           pOptDef->opt_def.f=dOptDef;   \
  pOptDef->opt_min.f=dOptMin;     pOptDef->opt_max.f=dOptMax;   \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_FLOAT_OPT(pOptDef,pShortOpt,pcLongOpt,pcArgName,pdVal,dOptDef,dOptMin,dOptMax,iMaxOcc,iItemSize,pcDesc) \
  PUT_OPT_DEF_FLOAT(pOptDef,pShortOpt,pcLongOpt,PO_REQUIRED_ARG,pcArgName,pdVal,dOptDef,dOptMin,dOptMax,1,0,pcDesc)

#define PUT_OPT_DEF_CALLBACK(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,pCallbackFunc,vOptDef,vOptMin,vOptMax,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otCallback; \
  pOptDef->val.cb=pCallbackFunc;  pOptDef->opt_def.v=vOptDef;   \
  pOptDef->opt_min.v=vOptMin;     pOptDef->opt_max.v=vOptMax;   \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_CALLBACK_OPT(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,pcbVal,vOptDef,vOptMin,vOptMax,iMaxOcc,iItemSize,pcDesc) \
  PUT_OPT_DEF_CALLBACK(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,pcbVal,vOptDef,vOptMin,vOptMax,1,0,pcDesc)

#define PUT_OPT_DEF_LIST(pOptDef,pShortOpt,pcLongOpt,iHasArg,pcArgName,pcbVal,vOptList,vOptDef,vOptMax,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otList;     \
  pOptDef->val.i=pcbVal;          pOptDef->opt_def.l=vOptDef;   \
  pOptDef->opt_min.l=vOptList;    pOptDef->opt_max.i=vOptMax;   \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_LIST_OPT(pOptDef,pShortOpt,pcLongOpt,pcArgName,pcbVal,vOptList,vOptDef,pcDesc) \
  PUT_OPT_DEF_LIST(pOptDef,pShortOpt,pcLongOpt,PO_REQUIRED_ARG,pcArgName,pcbVal,vOptList,vOptDef,0,1,0,pcDesc)

#define PUT_OPT_MULTIPIER(pOptDef,iHasArg,pcArgName,piItemsCnt,vArraySize,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=0;           pOptDef->long_opt=NULL;       \
  pOptDef->has_arg=iHasArg;       pOptDef->val_type=otMultipier;\
  pOptDef->val.i=piItemsCnt;      pOptDef->opt_def.l=0;         \
  pOptDef->opt_min.l=0;           pOptDef->opt_max.i=vArraySize;\
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }

#define PUT_OPT_SUBCMDLINE(pOptDef,pShortOpt,pcLongOpt,pcArgName,pOptDefs,iPoptOpts,pcDelims,iMaxOcc,iItemSize,pcDesc) \
{ pOptDef->short_opt=pShortOpt;   pOptDef->long_opt=pcLongOpt;  \
  pOptDef->has_arg=PO_REQUIRED_ARG; pOptDef->val_type=otSubcmd; \
  pOptDef->val.v=pOptDefs;        pOptDef->opt_def.i=iPoptOpts; \
  pOptDef->opt_min.c=pcDelims;    pOptDef->opt_max.i=0;         \
  pOptDef->desc=pcDesc;           pOptDef->arg_name=pcArgName;  \
  pOptDef->occ_max=iMaxOcc;       pOptDef->elem_size=iItemSize; \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }


#define PUT_OPT_DEF_END(pOptDef) \
{ pOptDef->short_opt=0;           pOptDef->long_opt=0;          \
  pOptDef->has_arg=PO_NO_ARG;     pOptDef->val_type=otNull;     \
  pOptDef->val.i=0;               pOptDef->opt_def.i=0;         \
  pOptDef->opt_min.i=0;           pOptDef->opt_max.i=0;         \
  pOptDef->desc=0;                pOptDef->arg_name=0;          \
  pOptDef->occ_max=0;             pOptDef->elem_size=0;         \
  pOptDef->occurences=0;                                        \
  pOptDef++;                                                    }
#define PUT_END_OPT(pOptDef) PUT_OPT_DEF_END(pOptDef)

enum popt_ret_codes
{
  POPT_RES_OK            =  0,
  ERR_POPT_ERROR         = -1, /**< General error */
  ERR_POPT_INVALID_PAR   = -2, /**< Invalid function parameters */
  ERR_POPT_MALLOC        = -3, /**< Memory allocation error */
  ERR_POPT_RANGE         = -4, /**< option`s or free parameter is ouside defined range  */
  ERR_POPT_CONVERSION    = -5, /**< error during parameter conversion  */
  ERR_POPT_UNKNOWN_OPT   = -6, /**< undefined option  */
  ERR_POPT_NO_REQ_PARAM  = -7, /**< required parameter not found  */
  ERR_POPT_UNKNOWN_PARAM = -8, /**< unknown or unexpected parameter found */
  ERR_POPT_SYNTAX        = -9  /**< syntax error */
} ;

enum parse_opt_options
{
  POPT_IGNORE_CASE             =  1,
   // UNDONE: POPT_ALLOW_ONE_DASH_FOR_LONG Unsupported yet
  POPT_ALLOW_ONE_DASH_FOR_LONG =  2, //< Allow using long name with only one -. This will block short opt syntax, like -c1
  POPT_NOT_USE_EQUAL_SIGN      =  4, //< Block allowing using = for parameter assigment (--opt=param)
  POPT_NOT_USE_COLON_SIGN      =  8, //< Block allowing using : for parameter assigment (--opt:param)
  POPT_REQ_SHORT_PAR_DELIM     = 16, //< Not allow for syntax like -c1, where short param is directly after opt
  POPT_NOTALLOW_UNPARSED_PARAMS= 32, //< Funcion will return error if numbers of free parameters exceed definined
  POPT_ALLOW_NAMED_PARAMS      = 64, //< Prameters given by their's name (eg filename=foo.bar) are allowed
#if 0
  POPT_TREAT_ALL_AS_OPTIONS    = 64, /**< Treat all parameters as options, even if they does not begin by '-'
                                          This option block using of unnamed and free parametres. It is used mainly for 
                                          parsing configuration files */
#endif
  POPT_QUIET                   = 256 //< Do not print error messages
} ;

/***********************************************************************************************
  Funkcja uogólnia funkcjê parsuj¹c¹ parametry lini poleceñ getopt_long()
  opts - tablica struktur opt_def okreslaj¹cych parametry, zakoñczona 
         pust¹ struktura.
***********************************************************************************************/
long int parse_opt(int argc, char* argv[],opt_defs_t *opts, unsigned options);



/* This fuction have to be defined outside */
void output_error_msg(const char* msg);

void _parseopt_error(const char* msg,...);

#ifdef	__cplusplus
}
#endif

#endif
