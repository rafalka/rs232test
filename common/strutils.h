/***************************************************************************
 * @file     strutils.h                                                     
 * @brief    Set of fuctions that supports operation on strings
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

#ifndef _STRUTILS_H_
#define _STRUTILS_H_

#include <string.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef	__cplusplus
extern "C" {
#endif

#if   defined(_MSC_VER)
# define STRCASECMP  _stricmp
# define STRNCASECMP _strnicmp
# define SNPRINTF    _snprintf
#elif defined(__GNUC__)
# define STRCASECMP  strcasecmp
# define STRNCASECMP strncasecmp
# define SNPRINTF    snprintf
#else
# error Portable string.h macros not defined for this platform
#endif


/**
 * Function do sprintf() to newly allocates buffer
 * 
 * @param[out]  out       Pointer to variable which will receive allocated buffer 
 * @param[in]   msg       Format specification
 * @param[in]   arglist   Pointer to list of arguments
 * 
 * @return  int   the number of characters written, not including the terminating null character, 
                  or a negative value if an output error occurs
 */
int vasprintf(char** out, const char* msg, va_list arglist);

/**
 * Function do sprintf() to newly allocates buffer
 * 
 * @param[out]  out       Pointer to variable which will receive allocated buffer 
 * @param[in]   msg       Format specification
 * @param[in]   ...       Pointer to list of arguments
 * 
 * @return  int   the number of characters written, not including the terminating null character, 
                  or a negative value if an output error occurs
 */
int asprintf(char** out, const char* msg,...);


/**
 * Function converts string to integer value using atoi() or returns badValue if strValue is empty
 * 
 * @param strValue text for convertion
 * @param badValue value returned in case strValue is empty
 * 
 * @return value returned by atoi() or badValue if strValue is empty
 */
int atoiEx(const char * strValue, int badValue); 

/**
 * Function returns address of first nonspace character (>' ')
 * 
 * @param s      string address
 * 
 * @return address of first nonspace character (>' ') or NULL if not found
 */
const char * lTrim(register const char * s);

/**
 * Function truncates all whitespace characters at the end of 
 * string by putting #0 char 
 * 
 * @param s      string address
 * 
 * @return s
 */
char * rTrim(char * s);

/**
 * Function truncates all whitespace characters from beggining
 * and end of string
 * 
 * @param s      string address
 * @param nSize  string size
 * 
 * @return address of first nonspace character (>' ') or NULL if
 *         not found
 */
char * rnTrim(char * s, size_t nSize);

/**
 * Function copies max <size> characters from <from> to <to> skips whitespaces at 
 * the end and begging
 * 
 * @param to     destination buffer
 * @param from   source string
 * @param size   max count of copied bytes
 * 
 * @return <to> or NULL if error
 */
char * StrNTrimCpy(char* to, const char* from, int size);

/**
 * Function truncates all whitespace characters from beggining
 * and end of string
 * 
 * @param from  string address
 * @param size  string size
 * 
 * @return address of first nonspace character (>' ') or NULL if
 *         not found
 */
char * lrnTrim(char* from, size_t size);

/**
 * Function test if passed string is not empty (contains non whitespace characters)
 * 
 * @param str    string
 * 
 * @return nonzero if string is not empty 
 */
char strNotEmpty(const char* str);
  
/**
 * Function is similiar to strncpy() but copies min(*tomaxsize,frommaxsize,strlen(from)) characters.
 * Function returns address to+copied count and decrease *tomaxsize by copied count
 * 
 * @param to        Destination buffer
 * @param tomaxsize size of <to> buffer
 * @param from      source string
 * @param frommaxsize
 *                  <from> string length
 * 
 * @return address to+copied count and decrease *tomaxsize by copied count
 */
char* StrMaxCpy(char* to, long *tomaxsize, const char* from, long frommaxsize);


/**
 * Function returns first nonspace char address and maximum count of nonspace string in *size
 * 
 * @param str    Source string
 * @param size   IN: maximum str size
 *               OUT: str size without whitespaces at the end
 * 
 * @return returns first nonspace char address  or NULL if not found
 */
char * get_trim_str(char* str, size_t* size);



/**
 * Function compares strsize of str with variable parameters list and return matched string index or -1
 * 
 * @param str     Source string
 * @param strsize source string size
 * 
 * @return matched string index or -1
 */
int str_is_in(char* str,size_t strsize,...);

/**
 * Function compares strsize of str with variable parameters list and return matched string index or -1 
 * Comparing is not case sensitive 
 * 
 * @param str     Source string
 * @param strsize Source string size
 * 
 * @return matched string index or -1
 */
int str_is_in_case(char* str,size_t strsize,...);

/**
 * Function converts <strsize> characters of <str>, with C-like 
 * formating symbols : 
 *   \n   NL   #13#10    new line
 *   \t   HT   #9        horizontal tabulation
 *   \v   VT   #11       vertical tabulation
 *   \b   BS   #8        backspace
 *   \l   LF   #10       new line
 *   \r   CR   #13       carriage return
 *   \f   FF   #12       page feed
 *   \a   BEL  #7        bell
 *   \0   NUL  #0        Null character
 *   \\   \    '\'       back slash
 *   \ooo ooo  #000      decimal value
 *   \xhh hh   #$hh      hexadecimal value
 *   \oOO oo   #0oo      octal value
 * na �a�cuch zwyk�y i zapisuje outstrsize znak�w w outstr
 * Because input string will be always longer than output 
 * string, str and outstr can overlap. 
 * 
 * @param str        Source string
 * @param strsize    Source string size
 * @param outstr     destination buffer
 * @param outstrsize Destination buffer size
 * 
 * @return length of string copied to outstr
 */
size_t CStrToStr(const char* str, size_t strsize, char* outstr, size_t outstrsize);

/**
 * Function splits <str> into parameters following bash shell splitting rules.
 * Funtion respect ' ' and " " characters. Within " " following formating will be used
 * formating symbols :
 *   \n   NL   #13#10    new line
 *   \t   HT   #9        horizontal tabulation
 *   \v   VT   #11       vertical tabulation
 *   \b   BS   #8        backspace
 *   \l   LF   #10       new line
 *   \r   CR   #13       carriage return
 *   \f   FF   #12       page feed
 *   \a   BEL  #7        bell
 *   \0   NUL  #0        Null character
 *   \\   \    '\'       back slash
 *   \ooo ooo  #000      decimal value
 *   \xhh hh   #$hh      hexadecimal value
 *   \oOO oo   #0oo      octal value
 * 
 * @param str    Source string
 * @param params OUT: allocated by malloc() array of parameters. Last parameter array will be null
 * 
 * @return parameters array size or -1 in case of errors
 */
int StrToParams(char* str, char*** params);


/**
 * This function parse Parameter String in format <name1>=<val1><DELIM><name1>=<val1>
 * ie: "vid=0xCAD,pid=0x6003, dev=1";
 * and return addresses and sizes of first key-value pair and pointer to rests of buffer
 * or NULL, if no parameters found.
 * 
 * @param text     Source string
 * @param delim    Delimeter char
 * @param oParName OUT: parameter name
 * @param oParNameLength
 *                 OUT: parameter name length
 * @param oParVal  OUT: value string
 * @param oParValLength
 *                 OUT: Value string length
 * 
 * @return  addresses and sizes of first key-value pair and pointer to rests of buffer
 *         or NULL, if no parameters found.
 */
char* GetParamFromText(const char* text, char delim,
                     char** oParName, unsigned long * oParNameLength,
                     char** oParVal,  unsigned long * oParValLength);

/**
 * This function performs "word wrap" at specified column passed in end_col.
 * 
 * @param str     String
 * @param end_col Column to wrap at
 * @param out_len output: wrapped line length
 * 
 * @return It returns pointer to next line and wrapped line length in out_len
 */
char* get_word_wrap(const char* str,
                    unsigned end_col, 
                    size_t* out_len);




typedef struct
{
  unsigned char  begc;          /**< char that begins section */
  unsigned char  endc;          /**< char that ends section */
  unsigned char  escc;          /**< escape char */
  unsigned char  subs_allowed;  /**< If non zero, subsections are allowed */
} str_sect_def_t;
/*
 Example of str_sect_def_t for GetStrSection()
 const   str_sect_def_t sdef[]= {{'{','}',0,1},{'(',')',0,1},{'"','"','\\',0},{'\'','\'','\\',0},{0} };
*/


/**
 * This function locates boundaries of section in text, delimiterd by given section definitions
 *             
 * @param[IN]  str               String to parse
 * @param[IN]  len               Maximum len of string to parse
 * @param[IN]  sect_defs         Array of section definitions, terminated by null definition (begc==0)
 *                               Because section char is a singe character, no more that 254 section definitios are possible
 * @param[OUT] outptr_sect_start pointer to variable that receive address of first section char (including section delim char)
 * 
 * @return Length of section or negative error code (-1 bad params, -2 nested sections limit exceeded)
 */
 int GetStrSection(const char* str, size_t len, const str_sect_def_t sect_defs[], const char** outptr_sect_start);


 /**
 * This function locates end of string delimeted by given set of delimeters, and optionally store pointer to 
 * first character of str after a group of delimeters.
 *             
 * @param[IN]  str               String to parse
 * @param[IN]  len               Maximum len of string to parse
 * @param[IN]  delim             String (null terminated) of delimeter characters
 * @param[OUT] op_next_str       optional pointer to variable that will receive address of first
 *                               character of str after a group of delimiters
 * 
 * @return Length of section or negative error code (-1 bad params)
 */
int GetStrDelim(const char* str, size_t len, const char* delims, const char** op_next_str);

/**
 * This function locates end of string delimeted by given set of delimeters, with preserving section definitions
 *             
 * @param[IN]  str               String to parse
 * @param[IN]  len               Maximum len of string to parse
 * @param[IN]  delim             String (null terminated) of delimeter characters
 * @param[IN]  sect_defs         Array of section definitions, terminated by null definition (begc==0)
 *                               Because section char is a singe character, no more that 254 section definitios are possible
 * @param[OUT] op_next_str       optional pointer to variable that will receive address of first
 *                               character of str after a group of delimiters
 * 
 * @return Length of section or negative error code (-1 bad params, -2 nested sections limit exceeded)
 */
int GetSectStrDelim(const char* str, size_t len, const char* delims, const str_sect_def_t sect_defs[], const char** op_next_str);



#ifdef	__cplusplus
}
#endif

#endif
