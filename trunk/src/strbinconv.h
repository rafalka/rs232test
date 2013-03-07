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

#ifndef STRBINCONV_H
#define STRBINCONV_H

#include <stdint.h>

#include <QString>
#include <QByteArray>


//======================================================= Utils Functs

QString TextToHtml(const char* str, size_t size);

uint8_t calcFCS(const QByteArray& buf);

//======================================================= Abstract prototypes
class QStrBinConv
{
protected:
    QStrBinConv() {}

public:
    enum VALIDITY {
        INVALID      = 0,
        VALID,
        MIGHT_VALID,
        BUF_TO_SMALL
    };
    virtual const char* getName() = 0;
    virtual VALIDITY    convert(QString& str, QByteArray* pOutBuf, int* pFailPosition) = 0;
    QByteArray  convert(QString& str)  { QByteArray buf; convert(str,&buf,NULL); return buf; }
    VALIDITY    validate(QString& str, int* pFailPosition=NULL) { return convert(str,NULL,pFailPosition); }
};

class QBinStrConv
{
protected:
    QBinStrConv()
        : addr_color("blue")
        , body_color("black")
        , supp_color("teal")
    { }
public:
    enum STR_FORMAT {
        PLAIN_TEXT,
        HTML
    };
    enum OUTB_OPTS
    {
      OUTB_NOT_SPECIFIED       =  0,
      OUTB_1B_REC              =  1,
      OUTB_2B_REC              =  2,
      OUTB_4B_REC              =  4,
      OUTB_8B_REC              =  8,
      OUTB_16B_REC             = 16,
      OUTB_32B_REC             = 32,
      OUTB_MAX_REC             = 64,
      OUTB_REC_MASK            = 0x000000FF,
      OUTB_BYTE                = 0x00000000,
      OUTB_WORD                = 0x00000100,
      OUTB_LONG                = 0x00000200,
      OUTB_LONGLONG            = 0x00000300,
      OUTB_TYPE_MASK           = 0x00000300,
      OUTB_ADDR_SIZE_AUTO      = 0x00000000,   /* size will be calculacted automatically */
      OUTB_ADDR_SIZE_MASK      = 0x0000F000,
      OUTB_SHOW_ADDR_DEC       = 0x00010000,
      OUTB_SHOW_ADDR_HEX       = 0x00030000, /* default */
      OUTB_SHOW_ADDR           = 0x00030000,
      OUTB_SHOW_HEX            = 0x00040000,
      OUTB_SHOW_ASCII          = 0x00080000,
      OUTB_FORCE_BE            = 0x00200000,
      OUTB_FORCE_LE            = 0x00300000,

      OUTB_SIMPLE       = OUTB_BYTE | OUTB_SHOW_HEX | OUTB_MAX_REC,
      OUTB_DEFAULT      = OUTB_16B_REC | OUTB_BYTE | OUTB_SHOW_ADDR_HEX | OUTB_ADDR_SIZE_AUTO | OUTB_SHOW_HEX | OUTB_SHOW_ASCII


    };
    QString addr_color;
    QString body_color;
    QString supp_color;
    virtual const char* getName() = 0;
    virtual QString convert(QByteArray& buf, STR_FORMAT format = PLAIN_TEXT, uint32_t options = OUTB_NOT_SPECIFIED ) = 0;
};

//=======================================================  asci2bin and bin2ascii
class QStr2AsciiBinConv : public QStrBinConv
{
protected:
    static const char* name;
    /*QAsciiBin2StrConv   QStr2AsciiBinConv() {}
    static QStr2AsciiBinConv _inst;
    static QStr2AsciiBinConv& instance() { return _inst; }*/
public:
    virtual const char* getName() { return name; }
    virtual QStrBinConv::VALIDITY convert(QString& str, QByteArray* pOutBuf, int*  );
};

class QAsciiBin2StrConv : public QBinStrConv
{
protected:
    static const char* name;
    /*
    QAsciiBin2StrConv() {}
    static QAsciiBin2StrConv _inst;
    static QAsciiBin2StrConv& instance() { return _inst; }*/
public:
    virtual const char* getName() { return name; }
    virtual QString     convert(QByteArray& buf, QBinStrConv::STR_FORMAT format, uint32_t);
};


//======================================================= hex2bin and bin2hex
class QHexStr2BinConv : public QStrBinConv
{
protected:
    static const char* name;
    /*QAsciiBin2StrConv   QStr2AsciiBinConv() {}
    static QStr2AsciiBinConv _inst;
    static QStr2AsciiBinConv& instance() { return _inst; }*/
public:
    virtual const char* getName() { return name; }
    virtual QStrBinConv::VALIDITY  convert(QString& str, QByteArray* pOutBuf, int* pFailPosition);
};


class QBin2HexStrConv : public QBinStrConv
{
private:
    static const unsigned OUTB_MAX_REC = 256;
public:
    enum  AddrTypes {
        NO_ADDR  = 0,
        ADDR_HEX = OUTB_SHOW_ADDR_HEX,
        ADDR_DEC = OUTB_SHOW_ADDR_DEC
    } addrType;
    unsigned  addrWidth;
    unsigned  recSize;
    unsigned  wordSize;
    bool  isBigEndian;
    bool  showHex;
    bool  showAscii;

protected:
    static const char* name;
    QString HexToStr( unsigned long address, const unsigned char* buf, unsigned long size,
                      QBinStrConv::STR_FORMAT format = QBinStrConv::PLAIN_TEXT,
                      uint32_t options = OUTB_NOT_SPECIFIED);
    /*
    QAsciiBin2StrConv() {}
    static QAsciiBin2StrConv _inst;
    static QAsciiBin2StrConv& instance() { return _inst; }*/
public:
    virtual const char* getName() { return name; }
    virtual QString convert(QByteArray& buf, QBinStrConv::STR_FORMAT format, uint32_t options);

    QBin2HexStrConv():
        addrType(ADDR_HEX),
        addrWidth(0),
        recSize(16),
        wordSize(1),
        isBigEndian(false),
        showHex(true),
        showAscii(true) {}
};

//======================================================= Converters collection
class QBinStrConvCollection
{
protected:
    static QBinStrConv* _convs[];
    //static QBinStrConv& _convs[];
public:
    typedef enum {
        CONV_HEX,
        CONV_ASCII,

        __CONV_CNT
    } converters_t;
    static int getCount( );
    static QBinStrConv* getConv(int index);
    static void disposeConv(QBinStrConv* conv){ (void)conv/* currently we don't create new instances of converter */; }
};

class QStrBinConvCollection
{
protected:
    static QStrBinConv* _convs[];
    //static QBinStrConv& _convs[];
public:
    typedef enum {
        CONV_HEX,
        CONV_ASCII,

        __CONV_CNT
    } converters_t;
    static int getCount( );
    static QStrBinConv* getConv(int index);
    static void disposeConv(QStrBinConv* conv){ (void)conv/* currently we don't create new instances of converter */; }
};

#endif // STRBINCONV_H
