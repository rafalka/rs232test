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

#ifndef CPPUTILS_H
#define CPPUTILS_H

#include <QScopedPointer>

template <typename T>
struct QScopedSubPointerDeleter
{
    static inline void cleanup(T *pointer)
    {
        // Enforce a complete pointer type.
        // If you get a compile error here, read the section on forward declared
        // classes in the QScopedPointer documentation.
        typedef char IsIncompleteType[ (sizeof(T)==sizeof(void*) ) ? 1 : -1 ];
        (void) sizeof(IsIncompleteType);
        T d = *pointer;
        *pointer = 0;
        delete d;
    }
};

#endif // CPPUTILS_H
