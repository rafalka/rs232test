/******************************************************************************
 * @file     QHelpers.cpp
 * 
 * @brief    
 * 
 * @date     04-12-2010
 * @author   Rafal Kukla
 ******************************************************************************
 *            Copyright (C) 2010 Rafal Kukla  ( rkdevel@gmail.com )
 *        This file is a part of rs232testng project and is released 
 *         under the terms of the license contained in the file LICENSE
 ******************************************************************************
 */

#include <ctype.h>


#include "QHelpers.h"

QStandardItem* genStandardItem(const QString& Title, const char* IconResPath, int ID)
{
    QStandardItem *item = new QStandardItem(Title);
    if (IconResPath)
    {
        QIcon* icon = new QIcon(IconResPath);

        if (icon)
        {
            item->setIcon(*icon);
        }
    }

    item->setData(ID,Qt::UserRole);

    return item;
}


QStandardItem* genStandardItemSeparator()
{
    QStandardItem *item = new QStandardItem();
    item->setData( QString::fromLatin1("separator"), Qt::AccessibleDescriptionRole );
    item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled) );

    return item;
}


QString TextToHtml(const char* str, size_t size)
{
#define outfunc(_str_,_size_) s+=QString::fromAscii(_str_,_size_)
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
