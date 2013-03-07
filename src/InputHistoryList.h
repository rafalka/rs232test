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

#ifndef INPUTHISTORYLIST_H
#define INPUTHISTORYLIST_H

#include <QVariantList>

class QByteArray;

class InputHistoryList
{
public:
    static unsigned max_depth;
    static void Add(QVariantList* storage, QByteArray* item);
private:
    explicit InputHistoryList() {}
    
};

#endif // INPUTHISTORYLIST_H
