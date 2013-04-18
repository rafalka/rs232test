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

#include <wctype.h>
#include "strbinconv.h"
#include "strutils.h"

QString TextToHtml(const char* str, size_t size)
{
#define outfunc(_str_,_size_) s+=QString::fromLatin1(_str_,_size_)
#define outsect() if (sect_start) outfunc(sect_start,str-sect_start); sect_start=NULL;

    QString s;
    register const char* sect_start = str;
    register const char* alias      = NULL;
    size_t               alias_len  = 0;
    register char c;
    char   tmpbuf[]="&#xFF;";


    while (size)
    {
        c=*str;
        switch(c)
        {
        case ' ': alias = "&nbsp;"; alias_len = 6; break;
        case '&': alias = "&amp;";  alias_len = 5; break;
        case '<': alias = "&lt;";   alias_len = 4; break;
        case '>': alias = "&gt;";   alias_len = 4; break;
        case '"': alias = "&quot;"; alias_len = 6; break;
        case 10:
        case 13:  alias = "<BR>\r\n"; alias_len = 6; break;
        default:
            if(isalpha(c))
            {
                if (!sect_start) sect_start = str;
                alias = NULL;
            }
            else
            {
                // Put as hex
                char b = ((unsigned char)c) >> 4; c&=0xF;
                // ugly opt ;-)
                tmpbuf[3]=b+ ((b<10)?'0':'A'-10);
                tmpbuf[4]=c+ ((c<10)?'0':'A'-10);
                alias = tmpbuf; alias_len = 6;
            }
        }
        if (alias)
        {
            outsect();
            outfunc(alias,alias_len);
        }

        str++; size--;
        // skip CRLF
        if ( (size) &&
             ( ((c==10)&&(*str==13)) ||
               ((c==13)&&(*str==10))
             )
           )
        {
            str++; size--;
        }
    }
    outsect();

    return s;
}

QString StrToCStrString(const char* str, size_t size)
{
    QString s;
    if ( str && size )
    {
        char     c;
        QChar    qc;
        QString  qs;

        s.reserve( size );
        qs.reserve(5);

        while (size--)
        {
            c = *str++;
            qc = c;
            if ( ! qc.isPrint())
            {
                switch (c)
                {
                case '\t': qs="\\t";break;
                case '\v': qs="\\v";break;
                case '\b': qs="\\b";break;
                case '\r': qs="\\r";break;
                case '\n': qs="\\n";break;
                case '\f': qs="\\f";break;
                case '\a': qs="\\a";break;
                case '\\': qs="\\\\";break;
                case '\0': qs="\\0";break;
                    // \?, \', \" skipped here. We'll allow to use this characters directly
                default:
                    qs=QString("\\x%1").arg(static_cast<ushort>(c),2,16,QLatin1Char('0') );
                }
                s += qs;
            }
            else
            {
                s += qc;
            }
        }

    }
    return s;
}

QString StrToCStrString(const QString& str)
{
    QString      s;
    const QChar* qc;
    char         c;

    int size = str.size();
    s.reserve( size );

    for ( qc = str.constData(); size>0; qc++, size--)
    {
        if ( qc->isPrint() )
        {
            s += *qc;
        }
        else
        {
            c = qc->toLatin1();
            switch ( c )
            {
            case '\t': s+="\\t";break;
            case '\v': s+="\\v";break;
            case '\b': s+="\\b";break;
            case '\r': s+="\\r";break;
            case '\n': s+="\\n";break;
            case '\f': s+="\\f";break;
            case '\a': s+="\\a";break;
            case '\\': s+="\\\\";break;
            case '\0': s+="\\0";break;
                // \?, \', \" skipped here. We'll allow to use this characters directly
            default:
                s+=QString("\\x%1").arg(static_cast<ushort>(c),2,16,QLatin1Char('0') );
            }
        }
    }
    return s;
}


//======================================================= calcFCS
uint8_t calcFCS(const QByteArray& buf)
{
    register const uint8_t* ptr = reinterpret_cast<const uint8_t*>( buf.constData() );
    register int cnt = buf.size();
    register unsigned int fcs=0;
    while (cnt--) fcs+=*ptr++;

    return ( 255 - (fcs&0xFF) );
}

//======================================================= QAsciiBin2StrConv
const char* QAsciiBin2StrConv::name = "Ascii";

QString QAsciiBin2StrConv::convert(QByteArray &buf, QBinStrConv::STR_FORMAT format,uint32_t)
{
    return ( format == QBinStrConv::HTML )
            ? TextToHtml( buf.data(), buf.size() )
            : QString::fromLocal8Bit( buf );
}


//======================================================= QStr2AsciiBinConv
const char* QStr2AsciiBinConv::name = "Ascii";

QStrBinConv::VALIDITY QStr2AsciiBinConv::convert(QString &str, QByteArray* pOutBuf, int*)
{
    if ( pOutBuf)
    {
        pOutBuf->append( str.toLocal8Bit() );
    }
    return QStrBinConv::VALID;
}


//======================================================= QStrBinConv
const char* QCStr2BinConv::name = "C-like formatted string";
QStrBinConv::VALIDITY QCStr2BinConv::convert(QString &str, QByteArray *pOutBuf, int *)
{
    if ( pOutBuf)
    {
        int strsize = str.size();
        if ( strsize > 0 )
        {
            pOutBuf->operator =( str.toLocal8Bit() );
            strsize = static_cast<int>( CStrToStr( pOutBuf->data(), strsize, pOutBuf->data(), strsize ) );
            pOutBuf->resize( strsize );
        }
        return QStrBinConv::VALID;
    }
    return QStrBinConv::BUF_TO_SMALL;
}


//======================================================= QBin2CStrConv
const char* QBin2CStrConv::name = "C-like formatted string";
QString QBin2CStrConv::convert(QByteArray &buf, QBinStrConv::STR_FORMAT format, uint32_t)
{
    QString s = StrToCStrString( QString::fromLocal8Bit(buf) );
    if (format == QBinStrConv::HTML)
    {
        s = s.toHtmlEscaped();
    }

    return s;
}

//======================================================= QBinStrConvCollection
QBinStrConv* QBinStrConvCollection::_convs[__CONV_CNT] =
{
    new QBin2HexStrConv(),
    new QAsciiBin2StrConv(),
    new QBin2CStrConv()
};


int QBinStrConvCollection::getCount()
{
    return sizeof(_convs)/sizeof(_convs[0]);
}

QBinStrConv* QBinStrConvCollection::getConv(int index)
{
    if ( (index<0) || (index>=getCount()) ) return NULL;
    return _convs[index];
}


//======================================================= QStrBinConvCollection
QStrBinConv* QStrBinConvCollection::_convs[__CONV_CNT] =
{
    new QHexStr2BinConv(),
    new QStr2AsciiBinConv(),
    new QCStr2BinConv()
};


int QStrBinConvCollection::getCount()
{
    return sizeof(_convs)/sizeof(_convs[0]);
}

QStrBinConv* QStrBinConvCollection::getConv(int index)
{
    if ( (index<0) || (index>=getCount()) ) return NULL;
    return _convs[index];
}


//======================================================= QBin2HexStrConv

/*****************************************************************************************************
_htos_be
 ***/
inline char* _htos_be(register const unsigned char* data, register char* buf,
register unsigned hlen)
{
  register unsigned char  c,b;
  while (hlen)
  {
        c = *data++;
        b = c >> 4; c&=0xF;
        *buf++=b+ ((b<10)?'0':'A'-10);
        *buf++=c+ ((c<10)?'0':'A'-10);

    hlen--;
  }

  return buf;
}

/*****************************************************************************************************
_htos_le
 ***/
inline char* _htos_le(register const unsigned char* data, register char* buf,
register unsigned hlen)
{
  register unsigned char  c,b;
  data+=hlen-1;
  while (hlen)
  {
        c = *data--;
        b = c >> 4; c&=0xF;
        *buf++=b+ ((b<10)?'0':'A'-10);
        *buf++=c+ ((c<10)?'0':'A'-10);

    hlen--;
  }

  return buf;
}

/***************************************************************************************************** negBufLE
 ***/
inline void negBufLE(register unsigned char* buf, register int bufsize)
{
    *buf = -*buf;
    while (--bufsize)
    {
        buf++;
        *buf = 0xFF-*buf;
    }
}

/***************************************************************************************************** negBufBE
 ***/
inline void negBufBE(register unsigned char* buf, register int bufsize)
{
    buf+=bufsize-1;
    *buf = -*buf;
    while (--bufsize)
    {
        buf--;
        *buf = 0xFF-*buf;
    }
}

/**
 Option definitions and support macros for hex_dump_ex()
*/
#define OUTB_ADDR_SIZE(_s_) ( ((_s_)<<12) & OUTB_ADDR_SIZE_MASK)
#define OUTB_REC_SIZE(_s_) ( (_s_) & OUTB_REC_MASK)

/* internal use only macros */
#define OUTB_GET_REC_SIZE(_opts_) ( (_opts_) & OUTB_REC_MASK)
#define OUTB_GET_ADDR_SIZE(_opts_) ( ((_opts_) & OUTB_ADDR_SIZE_MASK)>>12)
#define OUTB_GET_TYPE_SIZE(_opts_) ( 1<<(((_opts_) & OUTB_TYPE_MASK)>>8) )


/******************************************************************************************************
 * Function display data buffer in HEX and Ascii format
 *
 * @param[in]  address        start address
 * @param[in]  buf            data buffer
 * @param[in]  size           data size
 *
 */

QString QBin2HexStrConv::HexToStr( unsigned long address, const unsigned char* buf, unsigned long size,
                                   QBinStrConv::STR_FORMAT format,
                                   uint32_t options )
{
    static const char* sep     = "  ";
    static const char* eol     = "\n";

  QString head;
  QString tail;
  QString addrfmt = "%1:";
  QString hexfmt  = "%1";
  QString asciifmt= "%1";
  if (format==QBinStrConv::HTML)
  {
      head = "<pre>";
      tail = "</pre>";
      addrfmt  = QString("</b><font color=\"%1\">%2</font></b>").arg(addr_color).arg(addrfmt);
      hexfmt   = QString("<font color=\"%1\">%2</font>").arg(body_color).arg(hexfmt);
      asciifmt = QString("<font color=\"%1\"><code>%2</code></font>").arg(supp_color).arg(asciifmt);
  }

  char          hex[OUTB_MAX_REC*3+1];
  char          asc[OUTB_MAX_REC+1];
  unsigned long cnt,r;
  unsigned      typesize;
  AddrTypes addr_type;
  bool  is_big_endian;
  bool  show_hex;
  bool  show_ascii;
  unsigned      addrw,hexw,ascw,recw; /*Width of format str */
  int           sepRequired;


  if (options==OUTB_NOT_SPECIFIED)
  {
      addr_type = addrType;
      addrw = addrWidth;
      recw  = recSize;
      show_hex = showHex;
      show_ascii = showAscii;
      typesize = wordSize;
      is_big_endian = isBigEndian;
  }
  else
  {
      addr_type = static_cast<AddrTypes>(options & OUTB_SHOW_ADDR);
      addrw = OUTB_GET_ADDR_SIZE(options);
      typesize = OUTB_GET_TYPE_SIZE(options);
      recw  = OUTB_GET_REC_SIZE(options);
      show_hex   = (options & OUTB_SHOW_HEX) == OUTB_SHOW_HEX;
      show_ascii = (options & OUTB_SHOW_ASCII) == OUTB_SHOW_ASCII;
      if ( (options & OUTB_FORCE_BE) == OUTB_FORCE_BE)
      {
        is_big_endian = 1;
      }
      else if ( (options & OUTB_FORCE_LE) == OUTB_FORCE_LE)
      {
        is_big_endian = 0;
      }
      else /* determine automatically */
      {
        cnt = 0x01; // is unsigned long
        is_big_endian = ( *((unsigned char*) &cnt) != 0x01 );
      }

  }

  QString out = head;

  // Calculate address size
  if (addr_type != NO_ADDR)
  {
    if (!addrw)
    {
      if ( addr_type == ADDR_DEC )
      {
        addrw=1;
        for (cnt=address+size;cnt;cnt/=10) addrw++;
      }
      else if ( addr_type == ADDR_HEX )
      {
        addrw=0;
        for (cnt=address+size;cnt;cnt/=16) addrw++;
        // align to 2, 4, 8, 16, ...
        for (cnt=2; addrw>cnt; cnt<<=1) ;
        addrw = cnt;
      }
    }
  }

  // Calculate record size in bytes
  if (recw>OUTB_MAX_REC) recw=OUTB_MAX_REC;
  recw /= typesize;
  if (! recw) recw = 1;

  hexw  = recw*(typesize*2+1)-1;
  ascw  = recw;


  while (size>=typesize) /* only complete types */
  {
    r = size / typesize;
    if ( r > recw ) r = recw;

    sepRequired = 0;

    // Print address field
    if ( addr_type == ADDR_DEC )
    {
      //fprintf(out,"%*d: ",addrw,address);
        out+=QString(addrfmt).arg(address,addrw,10);
        sepRequired=1;
    }
    else if ( addr_type == ADDR_HEX )
    {
      //fprintf(out,"%.*X: ",addrw,address);
      out+=QString(addrfmt).arg(address,addrw,16,QChar('0'));
      sepRequired=1;
    }

    //Print hex field
    if ( show_hex )
    {
      char                *hptr = hex;
      const unsigned char *ptr  = buf;

      for (cnt=r;cnt;cnt--)
      {
        if (is_big_endian)
          hptr = _htos_be(ptr,hptr,typesize);
        else
          hptr = _htos_le(ptr,hptr,typesize);

        ptr  += typesize;
        if (cnt>1) *hptr++=' ';
      }

      *hptr=0;
      if (sepRequired) out+=sep;
      //fprintf(out,"%-*s ",hexw,hex);
      if (show_ascii)
        out+=QString(hexfmt).arg(hex,-hexw);
      else
          out+=QString(hexfmt).arg(hex);
      //out+=QString::fromLatin1(hex,hexw);

      sepRequired=1;
              //("%1 ").arg(static_cast<unsigned char>(hex),-addrw,16, QChar('0'));
    }

    //Print ascii field
    if ( show_ascii )
    {
      if ( typesize == sizeof(char) )
      {
        char                *aptr = asc;
        const unsigned char *ptr  = buf;

        for (cnt=r;cnt;cnt--)
            {
          *aptr++= iscntrl(*ptr) ? '.' : *ptr;
          ptr++;
        }

        *aptr = 0;

        if (sepRequired) out+=sep;
        //fprintf(out,"%-*s ",ascw,asc);
        if (format==QBinStrConv::HTML)
        {
            out+=QString(asciifmt).arg(TextToHtml(asc,r));
        }
        else
        {
            out+=QString(asciifmt).arg(asc,-ascw);
        }
        //out+=doc.toHtml();
        sepRequired=1;
      }
      else if ( typesize == sizeof(wchar_t) )
      {
        wchar_t       *aptr = (wchar_t*) asc;
        const wchar_t *ptr  = (const wchar_t*) buf;

        for (cnt=r;cnt;cnt--)
            {
          *aptr++= iswcntrl(*ptr) ? L'.' : *ptr;
          ptr++;
        }

        *aptr = 0;
        if (sepRequired) out+=sep;
        //fprintf(out,"%-*S ",ascw,asc);
        out+=QString(asciifmt).arg(QString::fromWCharArray((wchar_t*)asc, aptr-((wchar_t*)asc) ), -ascw);
        sepRequired=1;

      }
    }

    //fprintf(out,"\n");
    out+=eol;

    r *= typesize;
    buf     += r;
    address += r;
    size    -= r;
  }

  out+=tail;
  return out;
}

const char* QBin2HexStrConv::name = "HEX";

QString QBin2HexStrConv::convert(QByteArray &buf, QBinStrConv::STR_FORMAT format, uint32_t options)
{
    return HexToStr(0,reinterpret_cast<const unsigned char*>(buf.constData()),buf.size(), format, options );
}


//======================================================= QHexStr2BinConv
const char* QHexStr2BinConv::name = "HEX";

QStrBinConv::VALIDITY QHexStr2BinConv::convert(QString &str, QByteArray* pOutBuf, int *pFailPosition)
{
    QStrBinConv::VALIDITY res;


    const QChar *uc = str.unicode();
    int maxlen = str.length();
    int idx = 0;
    unsigned short code;
    unsigned char  val;
    bool is_nible = false;

    // Skip whitespace
    while (idx < maxlen && uc[idx].isSpace())
        ++idx;

    if (idx == maxlen)
        return QStrBinConv::MIGHT_VALID;

    res = QStrBinConv::VALID;
    val = 0;

    while (idx < maxlen)
    {
        const QChar &in = uc[idx];
        idx++;

        if (in.isSpace())
        {
            if (is_nible && pOutBuf) pOutBuf->append(val);
            is_nible = false;
            val = 0;
            continue;
        }
        code = in.unicode();

        if ( code >= 'A' && code <= 'F' ) {
            val = (val<<4) | (code - 'A'+10);
        } else if ( code>= 'a' && code <= 'f' ) {
            val = (val<<4) | (code - 'a'+10);
        } else if ( code>= '0' && code <= '9' ) {
            val = (val<<4) | (code - '0');
        }
        else // bad character
        {
            if (is_nible && pOutBuf) pOutBuf->append(val);
            res = QStrBinConv::INVALID;
            break;
        }
        if ( is_nible )
        {
            if (pOutBuf) pOutBuf->append(val);
            val = 0;
        }

        is_nible  = !is_nible;

    }
    if (is_nible && pOutBuf) pOutBuf->append(val);
    if (pFailPosition) *pFailPosition = idx;

    return res;
}

