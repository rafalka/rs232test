/***************************************************************************
 * @file     vfile.c
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "vfile.h"

#define  DYMMEM_REALLOC_HEAP 256

VFILE_RET _vf_dumb_write(VFILE* file, const char* data, size_t size, size_t* pdata_written);
VFILE_RET _vf_dumb_putc(register VFILE* file, register char chr);

VFILE_RET _vf_mem_write(VFILE* file, const char* data, size_t size, size_t* pdata_written);
VFILE_RET _vf_mem_putc(register VFILE* file, register char chr);

/*
======================================================================================
=== DUMB FILES
======================================================================================
*/


/***************************************************************************** _vf_dumb_write()
 ***/
VFILE_RET _vf_dumb_write(VFILE* file, const char* data, size_t size, size_t* pdata_written)
{
  (void) data; // This is a dumb write, so data isn't used ;-)
  file->offset+=size;
  if (pdata_written) *pdata_written=size;
  return VF_RET_OK;
}

/***************************************************************************** _vf_dumb_putc()
 ***/
VFILE_RET _vf_dumb_putc(register VFILE* file, register char chr)
{
  (void) chr; // This is a dumb write, so data isn't used ;-)
  file->offset++;
  return VF_RET_OK;
}

/***************************************************************************** vfopen()
 ***/
VFILE_RET vfopen(VFILE* out_vfile)
{
  if (!out_vfile) return VF_RET_BAD_PARAMS;
  
  memset(out_vfile,0,sizeof(*out_vfile));
  out_vfile->type = VF_TYPE_DUMB;
  out_vfile->write_proc = _vf_dumb_write;
  out_vfile->putc_proc = _vf_dumb_putc;
  return VF_RET_OK;
}






/*
======================================================================================
=== MEMORY FILES
======================================================================================
*/

/***************************************************************************** _vf_mem_write()
 ***/
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wall"
VFILE_RET _vf_mem_write(VFILE* file, const char* data, size_t size, size_t* pdata_written)
{
  VFILE_RET      res = VF_RET_OK;
  register char* p = file->ptr+file->offset;
  register char* d = (char*) data;

  if (!size) return VF_RET_OK;

  file->offset+=size;
  if (file->offset>file->size)
  {
    size-=file->offset-file->size;
    res = VF_RET_EOF;
  }

  if (data)
  {
    switch(size)
    {
    case 8: *p++=*d++; //lack of break here is not a bug
    case 7: *p++=*d++;
    case 6: *p++=*d++;
    case 5: *p++=*d++;
    case 4: *p++=*d++;
    case 3: *p++=*d++;
    case 2: *p++=*d++;
    case 1: *p++=*d++;
    default:
      memcpy(p,d,size);
    }
  }

  if (pdata_written) *pdata_written=size;
  return res;
}
//#pragma GCC diagnostic pop

/***************************************************************************** _vf_mem_putc()
 ***/

VFILE_RET _vf_mem_putc(register VFILE* file, register char chr)
{

  if (file->offset<file->size)
  {
    file->ptr[file->offset++]=chr;
    return VF_RET_OK;
  } 
  else 
  {
    file->offset++;
    return VF_RET_EOF;
  }

}


/***************************************************************************** vfopenm()
 ***/
VFILE_RET vfopenm(char* buf, size_t bufsize, VFILE* out_vfile)
{
  if (!out_vfile) return VF_RET_BAD_PARAMS;
  memset(out_vfile,0,sizeof(*out_vfile));
  out_vfile->type = (buf) ? VF_TYPE_MEM : VF_TYPE_DUMB;
  out_vfile->ptr  = buf;
  out_vfile->size = bufsize;
  out_vfile->write_proc = _vf_mem_write;
  out_vfile->putc_proc = _vf_mem_putc;
  return VF_RET_OK;
}

/*
======================================================================================
=== DYNAMIC MEMORY FILES
======================================================================================
*/
/***************************************************************************** _vf_dynmem_realloc()
 ***/
VFILE_RET _vf_dynmem_realloc(VFILE* file, size_t newsize)
{
  char* ptr; 
  newsize+=DYMMEM_REALLOC_HEAP;
  ptr=realloc(file->ptr,newsize);
  if (!ptr) return VF_RET_OUTOFMEM_ERR;
  file->ptr=ptr;
  file->size=newsize;
  return VF_RET_OK;
}
/***************************************************************************** _vf_dynmem_write()
 ***/
VFILE_RET _vf_dynmem_write(VFILE* file, const char* data, size_t size, size_t* pdata_written)
{
  VFILE_RET      res = VF_RET_OK;
  if ( ((file->offset+size) >= file->size)
       && ((res=_vf_dynmem_realloc(file,file->offset+size))!=VF_RET_OK )
     ) return res;

  return _vf_mem_write(file,data,size,pdata_written);
} 

/***************************************************************************** _vf_dynmem_putc()
 ***/
VFILE_RET _vf_dynmem_putc(register VFILE* file, register char chr)
{
  VFILE_RET      res = VF_RET_OK;
  if ( ((file->offset+1) >= file->size)
       && ((res=_vf_dynmem_realloc(file,file->offset+1))!=VF_RET_OK )
     ) return res;

  return _vf_mem_putc(file,chr);
}

/***************************************************************************** _vf_dynmem_close()
 ***/
void _vf_dynmem_close(VFILE* file)
{
  if (file->ptr) free(file->ptr);
  file->ptr    = NULL;
  file->size   = 0;
  file->offset = 0;
}
/***************************************************************************** vfopendm()
 ***/
VFILE_RET vfopendm(VFILE* out_vfile)
{
  if (!out_vfile) return VF_RET_BAD_PARAMS;
  memset(out_vfile,0,sizeof(*out_vfile));
  out_vfile->type = VF_TYPE_DYNMEM;
  out_vfile->write_proc = _vf_dynmem_write;
  out_vfile->putc_proc  = _vf_dynmem_putc;
  out_vfile->close_proc = _vf_dynmem_close;
  return VF_RET_OK;
}



/*
======================================================================================
=== C STREAM FILES
======================================================================================
*/

/***************************************************************************** _vf_file_write()
 ***/
VFILE_RET _vf_file_write(VFILE* file, const char* data, size_t size, size_t* pdata_written)
{
  VFILE_RET ret=VF_RET_OK;
  size_t    len=0;
  
  if (size) 
  {
    if (file->file)
    {
      len = fwrite(data,1,size,file->file);
      if (!len) ret=VF_RET_FILE_ERROR;
      else if (len<size) ret=VF_RET_EOF;
    } 
    else 
      len = size;
  }
  
  file->offset+=len;
  if (pdata_written) *pdata_written=(size_t) len;
  return ret;
}

/***************************************************************************** _vf_file_putc()
 ***/
VFILE_RET _vf_file_putc(register VFILE* file, register char chr)
{
  int r = fputc(chr,file->file);
  if (r==EOF) return VF_RET_FILE_ERROR;
  else if (!r)return VF_RET_EOF;
  file->offset++;
  return VF_RET_OK;
}

/***************************************************************************** vfopenf()
 ***/
VFILE_RET vfopenf(FILE* file, VFILE* out_vfile)
{
  if (!out_vfile) return VF_RET_BAD_PARAMS;
  memset(out_vfile,0,sizeof(*out_vfile));
  out_vfile->type = VF_TYPE_FILE;
  out_vfile->file = file;
  out_vfile->size = (size_t) (-1);
  out_vfile->write_proc = _vf_file_write;
  out_vfile->putc_proc  = _vf_file_putc;
  return VF_RET_OK;
}
/*
======================================================================================
=== COMMON FUNCTIONS
======================================================================================
*/


/***************************************************************************** vf_vprintf()
 *** 
 ***/
VFILE_RET vf_vprintf(VFILE* file, size_t* pdata_written, const char* Format,va_list arglist)
{
  VFILE_RET ret;
  int     len=0;


  if (!file) return VF_RET_BAD_PARAMS;

  ret = VF_RET_OK;

  switch(file->type)
  {
  case VF_TYPE_FILE: 
    len = vfprintf(file->file,Format,arglist);
    if (len<0) ret = VF_RET_FILE_ERROR;
    else file->offset+=len;
    break;
  case VF_TYPE_DYNMEM: 
    len = vsnprintf(NULL,0,Format,arglist);
    if ( ((file->offset+len) >= file->size)
         && ((ret=_vf_dynmem_realloc(file,file->offset+len))!=VF_RET_OK )
       ) break;
    //lack of break here is not a bug
  case VF_TYPE_MEM: 
    len = vsnprintf(file->ptr+file->offset,file->size-file->offset,Format,arglist);
    if (len<0) ret = VF_RET_FILE_ERROR;
    file->offset+=len;

    if (file->offset > file->size)
      ret = VF_RET_EOF;

    break;
  case VF_TYPE_DUMB: 
    len = vsnprintf(NULL,0,Format,arglist);
    file->offset+=len;
    break;
  }
  if (pdata_written) *pdata_written=(size_t) len;
  return ret;
}

/***************************************************************************** vf_printf()
 *** 
 ***/
VFILE_RET vf_printf(VFILE* file, size_t* pdata_written, const char* Format,...)
{
  VFILE_RET ret;

  va_list arglist;

  if (!file) return VF_RET_BAD_PARAMS;

  va_start(arglist,Format);
  ret = vf_vprintf(file,pdata_written,Format,arglist);
  va_end(arglist);
  return ret;
}

/***************************************************************************** vffillchar()
 ***/
VFILE_RET vffillchar(VFILE* file, char chr, size_t size, size_t* pdata_written)
{
  VFILE_RET ret;
  size_t    len=0;

  if (!file) return VF_RET_BAD_PARAMS;
  if (!size) return VF_RET_OK;
  if (size==1) 
  {
    ret=vfputchar(file,chr);
    len=(ret==VF_RET_OK)?1:0;
  } else {
    char    buf[512];
    size_t  s;
    
    memset( buf,chr,(size<sizeof(buf))?size:sizeof(buf) );
    while (size)
    {
      s=0;
      ret=vfwrite(file,buf, (size<sizeof(buf)) ? size : sizeof(buf),&s );
      len+=s; size-=s;
      if ( (ret!=VF_RET_OK) || (!s) ) break;
    }
    if (size && (ret==VF_RET_EOF) ) 
      file->offset+=size; //Move virtual offset beyond EOF

  }
  if (pdata_written) *pdata_written=len;
  return ret;

}

