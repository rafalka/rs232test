/***************************************************************************
 * @file     strutils.c
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

#include "strutils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>



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
int vasprintf(char** out, const char* msg, va_list arglist)
{
  int    buflen=0;
  if ( (buflen=vsnprintf(NULL,0,msg,arglist))>0 )
  {
    if ( out && (*out=(char*) malloc(buflen+2)) )
    {
      if ( (buflen=vsnprintf(*out,buflen+1,msg,arglist))>0 )
      {
        (*out)[buflen] = 0;
      }
      else
      {
        free(*out);
        *out = NULL;
      }
    }
  }
  return buflen;
}

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
int asprintf(char** out, const char* msg,...)
{
  int    buflen=0;
  va_list arglist;

  va_start(arglist,msg);
  buflen = vasprintf(out, msg, arglist);
  va_end(arglist);

  return buflen;
}

/**
 * Function converts string to integer value using atoi() or returns badValue if strValue is empty
 * 
 * @param strValue text for convertion
 * @param badValue value returned in case strValue is empty
 * 
 * @return value returned by atoi() or badValue if strValue is empty
 */
int atoiEx(const char * strValue, int badValue) 
{
    int nRetValue = badValue;

    if (strValue && strlen(strValue)) {
        nRetValue = atoi(strValue);
    }
    return nRetValue;
}

/**
 * Function returns address of first nonspace character (>' ')
 * 
 * @param s      string address
 * 
 * @return address of first nonspace character (>' ') or NULL if not found
 */
const char * lTrim(register const char * s) 
{
    if (s) 
		{
				while (*s && (*s<=' ')) s++;
				if (!*s) s=NULL;
		}
		return s;
}

/**
 * Function truncates all whitespace characters at the end of 
 * string by putting #0 char 
 * 
 * @param s      string address
 * 
 * @return s
 */
char * rTrim(char * s) {
    size_t i;

    if (s && (i = strlen(s) ))
    {
        while(i)
        {
            --i;
            if ( s[i]>0x20 ) break;
        }
        s[i + 1] = 0;
    }
    return s;
}

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
char * rnTrim(char * s, size_t nSize)
{
    if (!s || !nSize) return s;

    while (nSize && *s)
    {
        if (*s>=0x20 ) break;
        --nSize;
        ++s;
    }
    if (nSize && *s)
    {
        while(nSize)
        {
            --nSize;
            if ( s[nSize]>0x20 ) break;
        }
         s[nSize + 1] = 0;
    }

    return s;
}

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
char * StrNTrimCpy(char* to, const char* from, int size)
{
  int          slen,cnt;
  const char*  str;
  if ( (! to) || (! from) || (! size) ) return to;
  // Szukamy pocz¹tku
  for (str=from; ((*str)>0) && ((*str) <= 0x20); str++) ;
  //.. i koñca
  
  for (slen=0,cnt=0; str[cnt]>0 && cnt<=size; cnt++) if (str[cnt]>0x20) slen=cnt+1;
  strncpy(to,str,slen);
  to[slen]=0;
  return to;
}

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
char * lrnTrim(char* from, size_t size)
{
  size_t    slen,cnt;
  char*  str;
  if ( (! from) || (! size) ) return from;
  // Szukamy pocz¹tku
  for (str=from; ((*str)>0) && ((*str) <= 0x20); str++) ;
  //.. i koñca
  
  for (slen=0,cnt=0; str[cnt]>0 && cnt<=size; cnt++) {
    if (str[cnt]>0x20) slen=cnt+1;
    if (str!=from) from[cnt]=str[cnt];
  }
  from[slen]=0;
  return from;
}

/**
 * Function test if passed string is not empty (contains non whitespace characters)
 * 
 * @param str    string
 * 
 * @return nonzero if string is not empty 
 */
char strNotEmpty(const char* str)
// Funkcja sprawdza, czy ³añcuch nie jest pusty
{
  char res=0;
  if ( str) {
    for (; ((*str)>0)&&((*str) <= 0x20); str++) ;
    res=(*str>20);
  }
  return res;
}
  
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
char* StrMaxCpy(char* to, long *tomaxsize, const char* from, long frommaxsize)
{
  char* p;
  if (!to || !from || ! frommaxsize || ! (*tomaxsize) || !tomaxsize ) return to;
  if ( (p=(char*) memchr(from,0,frommaxsize)) ) frommaxsize=(long) (p-from);
  if (*tomaxsize<frommaxsize) frommaxsize=*tomaxsize;
  memmove(to,from,frommaxsize);
  if (*tomaxsize>frommaxsize) to[frommaxsize]='\0';
  *tomaxsize-=frommaxsize;
  return to+frommaxsize;
}



/**
 * Function returns first nonspace char address and maximum count of nonspace string in *size
 * 
 * @param str    Source string
 * @param size   IN: maximum str size
 *               OUT: str size without whitespaces at the end
 * 
 * @return returns first nonspace char address  or NULL if not found
 */
char * get_trim_str(char* str, size_t* size)
{
  size_t       slen,cnt;

  if (!str) return NULL;
  
  slen = (size && *size) ? *size : strlen(str);
  // Szukamy pocz¹tku
  for (cnt=0; ((*str)>0) && isspace(*str) && cnt<slen; str++,cnt++) ;
  if (!*str || cnt>=slen) return 0;

  //.. i koñca
  if (size) {
    slen-=cnt;
    for (cnt=0; str[cnt]>0 && cnt<=slen; cnt++) if (!isspace(str[cnt])) *size=cnt+1;
  }
  return str;
}




/**
 * Function compares strsize of str with variable parameters list and return matched string index or -1
 * 
 * @param str     Source string
 * @param strsize source string size
 * 
 * @return matched string index or -1
 */
int str_is_in(char* str,size_t strsize,...)
{
  va_list arglist;
  char*   arg;
  int     cnt,res=-1;
  size_t  s;

  va_start(arglist,strsize);
  for (cnt=0;(arg=va_arg(arglist,char*));cnt++)
  {
    s=strlen(arg);
    if ((strsize==s) && ! strncmp(str,arg,strsize)) {
      res=cnt;
      break;
    }
  }
  va_end(arglist);
  return res;
}

/**
 * Function compares strsize of str with variable parameters list and return matched string index or -1 
 * Comparing is not case sensitive 
 * 
 * @param str     Source string
 * @param strsize Source string size
 * 
 * @return matched string index or -1
 */
int str_is_in_case(char* str,size_t strsize,...)
{
  va_list arglist;
  char*   arg;
  int     cnt,res=-1;
  size_t  s;

  va_start(arglist,strsize);
  for (cnt=0; (arg=va_arg(arglist,char*)); cnt++)
  {
    s=strlen(arg);
    if ((strsize==s) && !STRNCASECMP(str,arg,strsize) )
    {
      res=cnt;
      break;
    }
  }
  va_end(arglist);
  return res;
}

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
 * to plain string and store it in output buffer
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
size_t CStrToStr(const char* str, size_t strsize, char* outstr, size_t outstrsize)
{
  const char  *strend;
  char        *outend;
  char        *bufpos=NULL;
  char        c;
  char        sign=0;

  if (!str || !strsize || !outstr || !outstrsize) return 0;

  strend=str+strsize;
  outend=outstr+outstrsize-1/*space for null char*/;
  bufpos=outstr;
  while (str<strend && bufpos<outend && *str)
  {
    c=*(str++);
    // Skip EOL chars - they must be provided explicit
    if (c=='\r')
    {
        c=*str;
        if (c=='\n') str++;
        continue;
    }
    else if (c=='\n')
    {
        c=*str;
        if (c=='\r') str++;
        continue;
    }

    if (c=='\\' && str<strend)
    {
      c=*(str++);
      switch (c)
      {
        case 'n': c=10;
//#ifdef WIN32
//          if (bufpos<outend) *(bufpos++)=13;
//#endif
                  break;
      case 't': c='\t';break;
      case 'v': c='\v';break;
      case 'b': c='\b';break;
      case 'r': c='\r';break;
        //case 'l': c=10;break;
      case 'f': c='\f';break;
      case '?': c='\?';break;
      case '\'': c='\'';break;
      case '\"': c='\"';break;
        case 'x':
          if (str<strend) {
            c=0;
            if (*str=='-') {sign=1; str++; } else sign=0;
            for(;str<strend && *str;str++) {
              if (*str>='0' && *str<='9') { c<<=4; c+=*str-'0'; }
              else if (*str>='a' && *str<='f') { c<<=4; c+=*str-'a'+10; }
              else if (*str>='A' && *str<='F') { c<<=4; c+=*str-'A'+10; }
              else break;
            }
            if (sign) c=-c;
          }
          break;
        case 'o': 
          if (str<strend) {
            c=0;
            if (*str=='-') {sign=1; str++; } else sign=0;
            for(;*str;str++) {
              if (*str>='0' && *str<='7') { c<<=3; c+=*str-'0'; }
              else break;
            }
            if (sign) c=-c;
          }
          break;
        default:
          if ((c>='0' && c<='9') || (c=='-')) {
            if (c=='-') sign=1; else {sign=0; str--;}
            c=0;
            for(;str<strend && *str;str++) {
              if (*str>='0' && *str<='9') { c*=10; c+=*str-'0'; }
              else break;
            }
            if (sign) c=-c;
          }
          break;
      }
    }
    if (c && bufpos<outend)
        *(bufpos++)=c;
    else
        break;

  }

  // We reserved space for null char
  *bufpos=0;

  return (size_t) (bufpos-outstr);
}

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
int StrToParams(char* str, char*** params)
{
  int         ressize=0,respos=0;
  char*       buf=NULL;
  size_t      buflen=0;
  char*       bufpos=NULL;
  char        c;
  char        dqmode=0;
  char        sign=0;
  
  if (!str || !params || !(buflen=strlen(str))) return -1;

  *params=NULL;
  do {
    if (!(buf= (char*) malloc(buflen))) {
      ressize=-1; break;
    }
    bufpos=buf;
    dqmode=0;
    for (; *str && *str<33; str++) ;
    if (! *str) break;
    while (*str) {
      c=*(str++);
      if (c<33) {
        if (!dqmode) break;
      } else if (c=='"') {dqmode=!dqmode; c=0; }
      else if (c=='\'') {
        if (!dqmode){ //Pominiecie jesli w cudzyslowiu
          c=0;
          while (*str && (*str!='\'')) *(bufpos++)=*(str++);
          if (*str) str++; else break;
        }
      } else if (c=='\\') {
        c=*(str++);
        switch (c){
          case 'n': c=10;
#ifdef WIN32
                    *(bufpos++)=13;
#endif
                    break;
          case 't': c=9;break;
          case 'v': c=11;break;
          case 'b': c=8;break;
          case 'r': c=13;break;
          case 'l': c=10;break;
          case 'f': c=12;break;
          case 'x': 
            c=0;
            if (*str=='-') {sign=1; str++; } else sign=0;
            for(;*str;str++) {
              if (*str>='0' && *str<='9') { c<<=4; c+=*str-'0'; }
              else if (*str>='a' && *str<='f') { c<<=4; c+=*str-'a'+10; }
              else if (*str>='A' && *str<='F') { c<<=4; c+=*str-'A'+10; }
              else break;
            }
            if (sign) c=-c;
            break;
          case 'o': 
            c=0;
            if (*str=='-') {sign=1; str++; } else sign=0;
            for(;*str;str++) {
              if (*str>='0' && *str<='7') { c<<=3; c+=*str-'0'; }
              else break;
            }
            if (sign) c=-c;
            break;
          default:
            if ((c>='0' && c<='9') || (c=='-')) {
              if (c=='-') sign=1; else {sign=0; str--;}
              c=0;
              for(;*str;str++) {
                if (*str>='0' && *str<='9') { c*=10; c+=*str-'0'; }
                else break;
              }
              if (sign) c=-c;
            }
            break;
        }
      }
      if (c) *(bufpos++)=c;
    }
    *bufpos=0;
    if (respos>=ressize) {
      char** t;
      ressize=respos+16;
      if (!(t=(char**) realloc((void*) *params,ressize*sizeof(**params)))){
        ressize=-1; break;
      }
      *params=t;
    }
    (*params)[respos++]=buf; buf=NULL;
  } while (str && *str);

  if (ressize>0) {
    char** t;
    ressize=(respos+1)*sizeof(**params);
    if (!(t=(char**) realloc((void*) *params,ressize))){
      if (*params) free(*params); *params=NULL; ressize=-2;
    } else {
      *params=t;
      (*params)[respos]=NULL;
    }
  } else {
    if (*params) free(*params); *params=NULL; 
  }

  if (buf) free(buf);
  return ressize;
}


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
                     char** oParVal,  unsigned long * oParValLength)
{
  char  *name=NULL, *name_end=NULL, *val=NULL,*val_end=NULL;
  char c;
  
  if (!text) return NULL;

  while ( (c=*text) && (c!='=') && (c!=delim) )
  {
    if (!isspace(c) ) 
    {
      if (!name) name = (char*) text;
      name_end = (char*) text;
    }
    text++;
  }
  if (*text=='=') text++;
  while ( (c=*text) && (c!='=') && (c!=delim) )
  {
    if (!isspace(c) ) 
    {
      if (!val) val = (char*) text;
      val_end = (char*) text;
    }
    text++;
  }
  if (*text==delim) text++;

  if (oParName)       *oParName=name;
  if (oParNameLength) *oParNameLength=(name)?(unsigned long) (name_end-name)+1:0;
  if (oParVal)        *oParVal=val;
  if (oParValLength)  *oParValLength=(val)?(unsigned long) (val_end-val)+1:0;

  return (char*) ((name || val)?text:NULL);
}

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
                    size_t* out_len)
{
  const char* str_start = str;
  const char *current_word=NULL,*last_word_end=NULL,*next_word_beg=NULL;
  if (!str) return NULL;
  /*if (start_col>=end_col) return NULL; //CHECK: it is good idea?
  // move pointer to start of column
  while (*str && start_col)
  {
    end_col--;
    str++;
  }*/
  // next_word_beg - 
  while (*str && end_col)
  {
    if ( (*str==10) || (*str==13) ){
      last_word_end = str;
      next_word_beg=NULL;
      break;
    } else if ( isspace(*str) ) {
      if (current_word) last_word_end=current_word+1;
      next_word_beg = NULL;
    } else {
      current_word=str;
      if (!next_word_beg) next_word_beg = str;
    }
    str++; end_col--;   
  }
  // in case if current_word ends just at req column
  if ( !*str || isspace(*str) ) 
  {
      if (current_word) last_word_end=current_word+1;
      next_word_beg = NULL;
  }

  if (!last_word_end)
  {
    //If word is longer than end_col
    last_word_end=str;
    next_word_beg=str;

  }
  else if ( (!next_word_beg || (next_word_beg<=last_word_end) ) && *str)
  {
    //skip whitespaces before word
    //Except CRLF
    while (*str && isspace(*str)) { 
      if ( (*str==13) || (*str==10) )
      {
        if ( (str[0]==13 && str[1]==10) || 
                (str[0]==10 && str[1]==13)
              ) str++;
        str++;
        break;
        /*
        if (crlf_found) break;
        else 
        {
          if ( (str[0]==13 && str[1]==10) || 
                  (str[0]==10 && str[1]==13)
                ) str++;
          crlf_found = 1;
        }*/
      }
      str++; 
    }
    next_word_beg=(*str)?str:NULL; //if end with space
  }
  if (out_len) *out_len = (size_t) ( last_word_end-str_start);
  return (char*) next_word_beg;
}




#define MAX_SUBS_LEVEL 256
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
 int GetStrSection(const char* str, size_t len, const str_sect_def_t sect_defs[], const char** outptr_sect_start)
{
#define _end_with_code(_code_) { result = _code_; goto _end; }
  int               result = -1;                 // Return result
  const char        *sstart=NULL, *send=NULL;    // Start and end section addresses
  unsigned char     sectidx[256];                // Table with idices of sect_defs per each ASCII character
  unsigned char     subsects[MAX_SUBS_LEVEL];    // Stack with all neted subsections
  int               level;                       // Current nest level
  int               sdefcnt;                     // Count of section_defs
  const char*       strend = str+len;            // End address of passed string
  unsigned char     c;                           // character under examination
  const             str_sect_def_t *sdef=NULL;   // Active section info

  if (!str || !sect_defs || (!len) ) _end_with_code( -1 );

  // Lets count passed sect_defs and prepare lookout table
  memset(sectidx,0, sizeof(sectidx) );
  for (sdefcnt=0; sdefcnt<255 && sect_defs[sdefcnt].begc; sdefcnt++) sectidx[sect_defs[sdefcnt].begc] = sdefcnt+1;
  if (!sdefcnt) _end_with_code(-1);

  level = 0; /* outside of any section */
  while (str<strend)
  {
    c = (unsigned char) *str;
    if (!level)
    {
      // No section started yet
      if (sectidx[c]) 
      {
        // Section found
        sstart = str;
        sdefcnt = sectidx[c]-1; /* Store real sect_def index */
        subsects[level] = sdefcnt;
        level++;
        sdef = &sect_defs[ sdefcnt ];
      }
    }
    else
    {
      // We're inside a section and we're looking for it's end
      
      if ( sdef->escc && (c == sdef->escc) )
      {
        // If defined escape char, and it found
        // Skip it and next character
        str+=2;
        continue;
      }
      if ( c == sdef->endc )
      {
        // We've found end of section
        if (! (--level) )
        {
          // If we went back to base level, we can stop our parsing
          send = str;
          break;
        }
        sdef = &sect_defs[ subsects[level-1] ];
      }
      else if (sdef->subs_allowed && sectidx[c] )
      {
        // We've found start of next subsection
        if ( level >= MAX_SUBS_LEVEL ) _end_with_code(-2); /* Maximum depth level reached */
        sdefcnt = sectidx[c]-1; /* Store real sect_def index */
        subsects[level] = sdefcnt;
        level++;
        sdef = &sect_defs[ sdefcnt ];
      }
    }

    str++;
  }
  // That's end
  result = ( sstart && send) ? (int)(send-sstart)+1 : 0;

  // Finalization
_end:

  if (outptr_sect_start) *outptr_sect_start = sstart;
  return result;
#undef _end_with_code
}

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
int GetStrDelim(const char* str, size_t len, const char* delims, const char** op_next_str)
{         
  int           result = -1;             // Return result
  unsigned char delimidx[256];           // Lookup array which stores information if ASCII char is an delimiter
  const char*   strend = str+len;        // End address of passed string

  if (!str || !delims || (!len) ) return -1;

  // prepare delims lookout table
  memset(delimidx,0, sizeof(delimidx) );
  while(*delims) delimidx[(unsigned)*delims++] = 1;

  // Find delimiter
  while (str<strend && !delimidx[(unsigned)*str] ) str++;

  result = (int)(len - (strend-str));
  if (op_next_str)
  {
    // Optional pointer which will receive address
    // of first char after group of all delimeters
    while (str<strend && delimidx[(unsigned)*str] ) str++;
    *op_next_str = (str<strend) ? str : NULL;
  }
  

  return result;
}


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
int GetSectStrDelim(const char* str, size_t len, const char* delims, const str_sect_def_t sect_defs[], const char** op_next_str)
{
  int               result = -1;                 // Return result
  unsigned char     delimidx[256];               // Lookup array which stores information if ASCII char is an delimiter
  unsigned char     sectidx[256];                // Table with indices of sect_defs per each ASCII character
  unsigned char     subsects[MAX_SUBS_LEVEL];    // Stack with all needed subsections
  int               level;                       // Current nest level
  int               sdefcnt;                     // Count of section_defs
  const char*       strend = str+len;            // End address of passed string
  unsigned char     c;                           // character under examination
  const             str_sect_def_t *sdef=NULL;   // Active section info


  if (!str || !delims || (!len) ) return -1;
  
  if (!sect_defs) return GetStrDelim(str,len,delims, op_next_str);

  // prepare delims lookout table
  memset(delimidx,0, sizeof(delimidx) );
  while(*delims) delimidx[(unsigned)*delims++] = 1;

  // Lets count passed sect_defs and prepare lookout table
  memset(sectidx,0, sizeof(sectidx) );
  for (sdefcnt=0; sdefcnt<255 && sect_defs[sdefcnt].begc; sdefcnt++) sectidx[sect_defs[sdefcnt].begc] = sdefcnt+1;
  if (!sdefcnt) return -1;

  level = 0; /* outside of any section */
  while (str<strend)
  {
    c = (unsigned char) *str;
    if (!level)
    {
      // No section started yet
      if (delimidx[c])
      {
        // Delimiter found
        break;
      }
      else if (sectidx[c]) 
      {
        // Section found
        sdefcnt = sectidx[c]-1; /* Store real sect_def index */
        subsects[level] = sdefcnt;
        level++;
        sdef = &sect_defs[ sdefcnt ];
      }
    }
    else
    {
      // We're inside a section and we're looking for it's end
      
      if ( sdef->escc && (c == sdef->escc) )
      {
        // If defined escape char, and it found
        // Skip it and next character
        str+=2;
        continue;
      }
      if ( c == sdef->endc )
      {
        // We've found end of section
        if ( --level )
        {
          sdef = &sect_defs[ subsects[level-1] ];
        }
      }
      else if (sdef->subs_allowed && sectidx[c] )
      {
        // We've found start of next subsection
        if ( level >= MAX_SUBS_LEVEL ) return -2; /* Maximum depth level reached */
        sdefcnt = sectidx[c]-1; /* Store real sect_def index */
        subsects[level] = sdefcnt;
        level++;
        sdef = &sect_defs[ sdefcnt ];
      }
    }

    str++;
  }

  result = (int)(len - (strend-str));
  if (op_next_str)
  {
    // Optional pointer which will receive address
    // of first char after group of all delimeters
    while (str<strend && delimidx[(unsigned)*str] ) str++;
    *op_next_str = (str<strend) ? str : NULL;
  }
  

  return result;
}
