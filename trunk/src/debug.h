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

#ifndef DEBUG_H
#define DEBUG_H

#define ASSERT_ALWAYS(_expr_) \
    if (! (_expr_) ) { \
        qFatal("Expression ( %s ) evaluated to false in %s:%d",#_expr_,__FILE__,__LINE__); \
    }

void displayErrorMessage(const QString& err);


#endif // DEBUG_H
