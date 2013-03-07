/***************************************************************************
 * @file     vfile.h
 * @brief    Definition of Virtual Files
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

#ifndef _VFILE_H_
#define _VFILE_H_

#include <stdio.h>

typedef enum {
  VF_RET_BAD_PARAMS = -1000,
  VF_RET_FILE_ERROR,
  VF_RET_OUTOFMEM_ERR,
  VF_RET_OK  = 0,
  VF_RET_EOF = 1
} VFILE_RET;

typedef enum {
  VF_TYPE_DUMB,
  VF_TYPE_MEM,
  VF_TYPE_FILE,
  VF_TYPE_DYNMEM
} VFILE_TYPE;



typedef struct _VFILE VFILE;

typedef VFILE_RET (__vf_write_proc_t)(VFILE* file, const char* data, size_t size, size_t* pdata_written);
typedef VFILE_RET (__vf_putc_proc_t)(register VFILE* file, register char chr);
typedef void (__vf_close_proc_t)(register VFILE* file);

struct _VFILE 
{
  VFILE_TYPE          type;
  union {
    FILE*      file;
    char*       ptr;
  };
  size_t       size;
  size_t       offset;
  __vf_putc_proc_t*    putc_proc;
  __vf_write_proc_t*   write_proc;
  __vf_close_proc_t*   close_proc;

};



#ifdef	__cplusplus
extern "C" {
#endif

VFILE_RET vfopen(VFILE* out_vfile);
VFILE_RET vfopenm(char* buf, size_t bufsize, VFILE* out_vfile);
VFILE_RET vfopendm(VFILE* out_vfile);
VFILE_RET vfopenf(FILE* file, VFILE* out_vfile);

#define vfwrite(file, data, size, pdata_written) (file)->write_proc( (file), (data), (size), (pdata_written) )
#define vfputchar(file, chr)                     (file)->putc_proc(  (file), (chr) )
#define vfclose(file)    if ((file)->close_proc) (file)->close_proc( (file) )
#define vfputs(file, string, pdata_written)      (file)->write_proc( (file), (string), strlen(string), (pdata_written) )
//#define vfseek(file, offset, origin)

VFILE_RET vffillchar(VFILE* file, char chr, size_t size, size_t* pdata_written);
VFILE_RET vf_printf(VFILE* file, size_t* pdata_written, const char* Format,...);
VFILE_RET vf_vprintf(VFILE* file, size_t* pdata_written, const char* Format,va_list arglist);

//VFILE_RET vfwrite(VFILE* file, char* data, size_t size, size_t* pdata_written);


#ifdef	__cplusplus
}
#endif

#endif
