/***************************************************************************
 * @file     printopt.h                                                     
 * @brief    Set of fuctions that supports formatting standard description
 *           for commandline options defined for use with parseopt API
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


#ifndef _PRINTOPT_H_
#define _PRINTOPT_H_


#include "vfile.h"
#ifdef	__cplusplus
extern "C" {
#endif

    /** 
 * Function prints into virtual file @file formatted description
 * of opt_val_listitem definitions. Output will be idented 
 * straring from @ident and wrapped at @max_column
 * 
 * @param file            virtual file descriptor
 * @param list            pointer to opt list items table
 * @param ident           starting column
 * @param max_column      ending column
 * @param bytes_written   OUT: poiter to variable that receives 
 *                        bytes written
 * 
 * @return int  <0 - ERROR, >=0 - OK          
 */
int vfprint_list_opts(VFILE* file, const opt_val_listitem_t* list, 
                   unsigned ident, unsigned max_column, size_t* bytes_written);


/**
 * Function print into virtual file @file formated description
 * of command line options defined in @opts. Output will be
 * wrapped at column @max_column. By
 * 
 * @param file       output virtual file description
 * @param opts       command line options definitions
 * @param max_column columnt to wrap at
 * @param bytes_written
 *                   OUT: pointer to variable that receives number bytes written
 * 
 * @return int <0 - ERROR, >0 - OK
 */
int vfprint_opts_desc(VFILE* file, const opt_defs_t* opts, unsigned max_column,
                      size_t* bytes_written);


#ifdef	__cplusplus
}
#endif

#endif
