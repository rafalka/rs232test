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



#include <QByteArray>
#include "InputHistoryList.h"

unsigned InputHistoryList::max_depth = 20;

void InputHistoryList::Add(QVariantList *storage, QByteArray *item)
{
    if (! storage || !item) return;

    // Looking for duplicates
    int i;
    for (i = 0; i < storage->size(); ++i)
    {
        const QVariant& n = storage->at(i);
        if ( ! n.canConvert(QVariant::ByteArray) ) continue;

        if (n.toByteArray() == *item) break;
    }
    if ( i < storage->size() )
    {
        // Item already exists, just move it to front
        storage->move(i,0);
    }
    else
    {
        // Not found - let's add it
        storage->push_front(*item);
    }

    // Control capacity
    if ( ( i = storage->size()) > (int)max_depth )
    {
        //storage->   max_depth
        i-=max_depth;
        while (i--) storage->removeLast();
    }
}

