/***************************************************************************
 * @file     printopt.c
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

#include <string.h>

#include "parseopt.h"
#include "printopt.h"
#include "strutils.h"

#define OPT_NAME_CHAR   "string"
#define OPT_NAME_STRING "C string"
#define OPT_NAME_INT    "number"
#define OPT_NAME_FLOAT  "fpoint val"
#define OPT_NAME_BOOL   "bool"
#define OPT_NAME_CALLB  "param"
#define OPT_NAME_LIST   "val"
#define OPT_IDENT_SPACES    2
#define OPT_MIN_DESC_LEN    20
#define OPT_RANGE_FMT_INT    "Range: <%d..%d>\n"
#define OPT_RANGE_FMT_FLOAT  "Range: <%f..%f>\n"
#define OPT_RANGE_FMT_BOOL   "Possible values: " OPT_BOOL_VALS "\n"
#define OPT_RANGE_FMT_STRING "Parameter can include C-like special characters\n"
#define OPT_RANGE_FMT_LIST   "Possible <%s> values: \n"

#if defined WIN32 || defined _WIN32
#define  _CRLF_LEN 2
#define _PUT_CRLF(_ptr_) {*_ptr_++=13; *_ptr_++=10; }
#else   
#define  _CRLF_LEN 1
#define _PUT_CRLF(_ptr_) {*_ptr_++=10; }
#endif   
/* FORMAT:
  -x, --yyy             DESCRIPTION - opt without val
                        DESCRIPTION line 2
  -x, --yyy[=value]     DESCRIPTION - opt with optional val
  -x, --yyy=value       DESCRIPTION - opt with required val
  -x [value]            DESCRIPTION - short opt with optional val
  -x <value>            DESCRIPTION - short opt with required val
  --yyy=[value]         
*/
/*
return: <0 - error
         0 - ok
        >0 - buffer to small
*/


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
 *                        bytes writted
 * 
 * @return int  <0 - ERROR, >=0 - OK          
 */
int vfprint_list_opts(VFILE* file, const opt_val_listitem_t* list, 
                   unsigned ident, unsigned max_column, size_t* bytes_written )
{
  VFILE_RET  ret=VF_RET_FILE_ERROR;
  size_t     pol=0,cl,tol=0;
  const      opt_val_listitem_t* item;
  const char *str,*ptr;
  size_t     line;
  size_t     written, total=0;

#define _PUTCHAR(_chr_)         if ( (ret = vfputchar(file, (_chr_))) < 0 ) goto _vfprint_list_opts_ret; else total++
#define _WRITE(_data_,_size_)   if ( (ret = vfwrite(file,((const char*) _data_),(_size_),&written)) < 0 ) goto _vfprint_list_opts_ret; else total+=written
#define _WRITESTR(_str_)        if ( (ret = vfwrite(file,((const char*) _str_),strlen(_str_),&written)) < 0 ) goto _vfprint_list_opts_ret; else total+=written
#define _FILLCHAR(_chr_,_size_) if ( (ret = vffillchar(file,(_chr_),(_size_),&written)) < 0 ) goto _vfprint_list_opts_ret; else total+=written
#if defined WIN32 || defined _WIN32
#define _PUTCRLF { _PUTCHAR(13); _PUTCHAR(10); }
#else   
#define _PUTCRLF { _PUTCHAR(10); }
#endif   

  if ( !list || !file ) return -1;
  if (!max_column) max_column=65535; // Set column size to a big value
  else if (max_column<8) return -2; // column size should have "real" length
  //else max_column-=_CRLF_LEN;       // reserve space for CRLF char at EOL
  if (ident+8>=max_column) return -3;
  
  // STEP1: calculate max size Name column
  item = list; pol = 0;
  while (item->Name) 
  {
    cl = strlen(item->Name);
    if (pol<cl) pol=cl;
    item++;
  }
  pol++; //Add space after name
  
  tol = ident + (( (pol+ident)<(max_column-10) ) ? pol : 1 );

  // STEP2: print description 
  item = list; 
  while (item->Name) 
  {
    line = file->offset;
    _FILLCHAR(' ',ident);
    _WRITESTR(item->Name);
    if (item->Desc)
    {
      str = item->Desc;
      while (str)
      {
        // Put indent spaces
        if ( file->offset< (line+tol) ) {
          _FILLCHAR(' ',line+tol-file->offset); // fill spaces up to col length
        } else {
          _WRITESTR(" -");
        }

        //TODO: vfprint_list_opts(): What if parameters exceed max_column?
        if ( (file->offset-line) < (max_column-1) )
        {
          ptr = get_word_wrap(str,(unsigned) (max_column - (size_t) (file->offset-line)),&cl);
          if (!cl) {
            _PUTCRLF;
            break;
          }
          _WRITE(str,cl); //copy description
        } else ptr=str;
        
        // Add EOL characters;
        _PUTCRLF;
        line=file->offset;
        if (!ptr) break;
        str = ptr;
      }
    }
    item++;
  }

_vfprint_list_opts_ret:
  if (bytes_written) *bytes_written = total;

  return ret;
#undef _PUTCHAR
#undef _WRITE
#undef _WRITESTR
#undef _FILLCHAR
#undef _PUTCRLF
}

//TODO: gen_opts_desc() REMOVE ME!!!
int gen_opts_desc(const opt_defs_t* opts, unsigned max_column,
                  char* buf, size_t bufsize, size_t* bytes_written)
{
  size_t pol=0,cl,tol=0,total=0;
  const opt_defs_t* op;
  const char *str,*line,*ptr;
  char* buf_start=buf;
  int   i,cnt,opts_len=0;

  if (!opts) return -1;
  if (!max_column) max_column=65535; // Set column size to a big value
  else if (max_column<8) return -2; // column size should have "real" length
  else max_column-=_CRLF_LEN;       // reserve space for CRLF char at EOL

  
  op = opts;
  while ( op->short_opt || op->long_opt || op->has_arg || op->val.v) { opts_len++; op++;}

  if (!opts_len) {
    if (bytes_written) *bytes_written = 0;
    return 0;
  }
  //opts_len--;


  // STEP1: calculate max size of each column
  for (cnt=opts_len,op = opts;cnt;cnt--)
  {
    if (op->short_opt || op->long_opt)
    {
      cl = ( op->short_opt)?2:0; //This always will be somethink like -x
      if (op->long_opt) cl+=strlen(op->long_opt)+2; //format: --xxxxx
      if (op->has_arg==PO_OPTIONAL_ARG || op->has_arg==PO_REQUIRED_ARG)
      {
        switch(op->val_type)
        {
          case   otChar:     pol = sizeof(OPT_NAME_CHAR)+1; break;
          case   otString:   pol = sizeof(OPT_NAME_STRING)+1; break;
          case   otInt:      pol = sizeof(OPT_NAME_INT)+1; break;
          case   otFloat:    pol = sizeof(OPT_NAME_FLOAT)+1; break;
          case   otBool:     pol = sizeof(OPT_NAME_BOOL)+1; break;
          case   otCallback: pol = sizeof(OPT_NAME_CALLB)+1; break;
          case   otList:     pol = sizeof(OPT_NAME_LIST)+1; break;   
          default:
            pol=0;
        }
      } else pol=0;
      cl+=pol;
      
      if (op->short_opt && op->long_opt) cl+=2; // add ", " between short and long opt
      cl+=1;             // add "=" between long opt and value or space between short and val
      
       //Add space for braches - [] for optional param, or <> for short opt
      if (pol && ((op->has_arg==PO_OPTIONAL_ARG) || !op->long_opt)) cl+=2;
      if (cl) cl+=OPT_IDENT_SPACES+1; // Add indent at beggining, and one space befor desc
      
      if (tol<cl) tol = cl;
    }
    op++;
  }
  // Check if we'll have enough space for description
  if (max_column-tol<OPT_MIN_DESC_LEN) 
    tol =(max_column<OPT_MIN_DESC_LEN+2*OPT_IDENT_SPACES) 
         ? 2*OPT_IDENT_SPACES : max_column-OPT_MIN_DESC_LEN;

  pol=max_column-tol; //Desc max width
  //STEP2: Calculate total buffer needed for output string    
  total=0;
  for (cnt=opts_len,op = opts;cnt;cnt--)
  {
    if (op->short_opt || op->long_opt)
    {
      if (op->desc) 
      {
        total+=tol;
        str = op->desc;
        while ( (str = get_word_wrap(str,(unsigned) pol,&cl)) ) total+=tol+cl+_CRLF_LEN;
        total+=tol+cl+_CRLF_LEN; //Add last line
      } else 
        total+=+_CRLF_LEN;
    }
    op++;
  }
  if (!buf || bufsize<total) {
    if (bytes_written) *bytes_written=total;
    return 1;
  }
  //STEP3: Fill output buffer
  op = opts; total=0;
  for (cnt=opts_len,op = opts;cnt;cnt--)
  {
    line = buf;
    if (op->short_opt || op->long_opt)
    {
      //Put indent 
      for (i=OPT_IDENT_SPACES;i;i--) *buf++=' ';

      if (op->short_opt) {
        *buf++='-';
        *buf++=op->short_opt;
      }
      if (op->short_opt && op->long_opt) {
        *buf++=',';
        *buf++=' ';
      }
      if (op->long_opt)
      {
        cl=strlen(op->long_opt);
        *buf++='-'; *buf++='-';
        strncpy(buf,op->long_opt,cl);
        buf+=cl;
      }
      if (op->has_arg==PO_OPTIONAL_ARG || op->has_arg==PO_REQUIRED_ARG)
      {
        switch(op->val_type)
        {
          case   otChar:     str=OPT_NAME_CHAR; break;
          case   otString:   str=OPT_NAME_STRING; break;
          case   otInt:      str=OPT_NAME_INT; break;
          case   otFloat:    str=OPT_NAME_FLOAT; break;
          case   otBool:     str=OPT_NAME_BOOL; break;
          case   otCallback: str=OPT_NAME_CALLB; break;
          case   otList:     str=OPT_NAME_LIST; break;   
          default:
            str=NULL;
        }
      } else str=NULL;

      if (str)
      {
        if (!op->long_opt)*buf++=' '; //put space between short opt and val
        // Add braces for optional arg or for required if only short opt was given
        if (op->has_arg==PO_OPTIONAL_ARG) *buf++='[';
        else if (!op->long_opt) *buf++='<';
        if (op->long_opt)*buf++='=';  // add "=" between long opt and value
        cl=strlen(str);
        strncpy(buf,str,cl);
        buf+=cl;

        // Add braces for optional arg or for required if only short opt was given
        if (op->has_arg==PO_OPTIONAL_ARG) *buf++=']';
        else if (!op->long_opt) *buf++='>';
      }
  
      // Add description
      if (op->desc)
      {
        str = op->desc;
        while (str)
        {
          ptr=line+tol;
          while (buf<ptr)*buf++=' ';// fill spaces up to col length
          if (buf[-1]!=' ')*buf++=' ';
          //TODO: gen_opts_desc(): What if parameters exceed max_column?

          ptr = get_word_wrap(str,(unsigned) (max_column - (size_t) (buf-line)),&cl);
          if (!cl) break;
          strncpy(buf,str,cl); buf+=cl; //copy description
          
          // Add EOL characters;
          _PUT_CRLF(buf);
          line=buf;
          if (!ptr) break;
          str = ptr;
        }
      } else _PUT_CRLF(buf);
    }

    op++;
  }

  if (bytes_written) *bytes_written=buf-buf_start;

  return 0;
}

static const char* _get_oparg_name(const opt_defs_t* op)
{
	const char* str;
	if (op->has_arg==PO_OPTIONAL_ARG || op->has_arg==PO_REQUIRED_ARG)
	{
		if (op->arg_name) str=op->arg_name;
		else
		{
			switch(op->val_type)
			{
				case   otChar:     str=OPT_NAME_CHAR; break;
				case   otString:   str=OPT_NAME_STRING; break;
				case   otInt:      str=OPT_NAME_INT; break;
				case   otFloat:    str=OPT_NAME_FLOAT; break;
				case   otBool:     str=OPT_NAME_BOOL; break;
				case   otCallback: str=OPT_NAME_CALLB; break;
				case   otList:     str=OPT_NAME_LIST; break;   
				default:
					str=NULL;
			}
		}
	} else str=NULL;

	return str;
}


#if defined WIN32 || defined _WIN32
#define _PUTCRLF { _PUTCHAR(13); _PUTCHAR(10); }
#define _PUTOUTCRLF { _PUTOUT(13); _PUTOUT(10); }
#else   
#define _PUTCRLF { _PUTCHAR(10); }
#define _PUTOUTCRLF { _PUTOUT(10); }
#endif   

static VFILE_RET _print_range_desc(VFILE* vfbuf, const opt_defs_t * op,unsigned max_column) 
{
  VFILE_RET  ret=VF_RET_OK;
	const char * str;
#define _WRITESTR(_str_)         if ( (ret = vfwrite(vfbuf,((const char*) _str_),strlen(_str_),NULL)) < 0 ) goto _print_range_desc_end
#define _PUTCHAR(_chr_)          if ( (ret = vfputchar(vfbuf, (_chr_))) < 0 ) goto _print_range_desc_end

	if (op->has_arg==PO_OPTIONAL_ARG || op->has_arg==PO_REQUIRED_ARG)
	{
		switch(op->val_type)
		{
			case   otString:   
				str=OPT_RANGE_FMT_STRING; 
				_WRITESTR(str);
				break;
			case   otInt:      
				if ( (ret = vf_printf(vfbuf,NULL,OPT_RANGE_FMT_INT,
											op->opt_min.i,op->opt_max.i)) < 0 
					 ) goto _print_range_desc_end;
				break;
			case   otFloat:
				if ( (ret = vf_printf(vfbuf,NULL,OPT_RANGE_FMT_FLOAT,
											op->opt_min.f,op->opt_max.f)) < 0 
					 ) goto _print_range_desc_end;
				break;
			case   otBool:
				str=OPT_RANGE_FMT_BOOL; 
				_WRITESTR(str);
				break;
			case   otList:
        if ( (ret = vf_printf(vfbuf,NULL,OPT_RANGE_FMT_LIST,
											op->arg_name) ) < 0 
					 ) goto _print_range_desc_end;
				if ( (ret = vfprint_list_opts(vfbuf,op->opt_min.l,1,max_column,NULL))<0
					 ) goto _print_range_desc_end;
				break;
			default:
				// If CRLF haven't put by description
				if (!op->desc)_PUTCRLF;
		}
	} 
	_PUTCHAR('\0'); //Put string terminator
_print_range_desc_end:
	return ret;
#undef _WRITESTR
#undef _PUTCHAR
}

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
                      size_t* bytes_written)
{
  size_t     pol=0,cl;
  size_t     tol=0; /* Option column width */
  size_t     tpl=0; /* Parameter column width */
  const      opt_defs_t* op;
  const char *str,*ptr;
  size_t     line,buf_start;
  int        i,cnt,opts_len=0;
  VFILE_RET  ret=VF_RET_FILE_ERROR;
  VFILE      vfbuf;
#define _WRITEOUT(_data_,_size_) if ( (ret = vfwrite(file,((const char*) _data_),(_size_),NULL)) < 0 ) goto _vfprint_opts_desc_end
#define _FILLOUT(_chr_,_size_)   if ( (ret = vffillchar(file,(_chr_),(_size_),NULL)) < 0 ) goto _vfprint_opts_desc_end
#define _PUTOUT(_chr_)           if ( (ret = vfputchar(file, (_chr_))) < 0 ) goto _vfprint_opts_desc_end
#define _WRITESTROUT(_str_)      if ( (ret = vfwrite(file,((const char*) (_str_)),strlen(_str_),NULL)) < 0 ) goto _vfprint_opts_desc_end

#define _PUTCHAR(_chr_)          if ( (ret = vfputchar(&vfbuf, (_chr_))) < 0 ) goto _vfprint_opts_desc_end
#define _FILLCHAR(_chr_,_size_)   if ( (ret = vffillchar(&vfbuf,(_chr_),(_size_),NULL)) < 0 ) goto _vfprint_opts_desc_end
#define _WRITE(_data_,_size_)    if ( (ret = vfwrite(&vfbuf,((const char*) _data_),(_size_),NULL)) < 0 ) goto _vfprint_opts_desc_end
#define _WRITESTR(_str_)         if ( (ret = vfwrite(&vfbuf,((const char*) _str_),strlen(_str_),NULL)) < 0 ) goto _vfprint_opts_desc_end

  if (!opts || !file) return -1;
  if (!max_column) max_column=65535; // Set column size to a big value
  else if (max_column<8) return -2; // column size should have "real" length
  //else max_column-=_CRLF_LEN;       // reserve space for CRLF char at EOL

  vfopendm(&vfbuf);
  op = opts;
  while ( op->short_opt || op->long_opt || op->has_arg || op->val.v) { opts_len++; op++;}

  if (!opts_len) {
    if (bytes_written) *bytes_written = 0;
    return 0;
  }
  //opts_len--;

	/************************************************************************************
	* PART1: Print information about parameters
	*/
  // STEP1: calculate max size of each column
	//  we have two types: parameters and options, and they shold be algined separately 
  for (cnt=opts_len,op = opts;cnt;cnt--)
  {
		//Argument name size
		str = _get_oparg_name(op);
		pol = (str) ? strlen(str) : 0;
		cl=OPT_IDENT_SPACES+pol+1;                     // Add indent at beggining, and one space befor desc
		if (op->short_opt) cl+=2;                      // This always will be somethink like -x
		if (op->long_opt)  cl+=strlen(op->long_opt)+2; // format: --xxxxx
		if (op->short_opt && op->long_opt) cl+=2;      // add ", " between short and long opt

		//Add space for braches - [] for optional param, or <> for short opt or parameter
		if (pol && ((op->has_arg==PO_OPTIONAL_ARG) || !op->long_opt)) cl+=2;

		if (op->short_opt || op->long_opt) //It's an option
    {
			if (pol) cl+=1;                  // add "=" between long opt and value or space between short and val
      if (tol<cl) tol = cl;
    } else {                           //It's a parameter
			if (tpl<cl) tpl = cl;
		}
    op++;
  }
  // Check if we'll have enough space for description
  if ( (int)(max_column-tol) < OPT_MIN_DESC_LEN ) 
    tol = ( max_column < (2*OPT_IDENT_SPACES+OPT_MIN_DESC_LEN) ) 
          ? (2*OPT_IDENT_SPACES) : (max_column-OPT_MIN_DESC_LEN);

	if ( (int)(max_column-tpl) < OPT_MIN_DESC_LEN ) 
		tpl = ( max_column < (2*OPT_IDENT_SPACES+OPT_MIN_DESC_LEN) ) 
					? (2*OPT_IDENT_SPACES) : (max_column-OPT_MIN_DESC_LEN);

	pol=max_column-tol; //Desc max width

	// STEP2: Print Parameters descriptions
	op = opts; buf_start=0;
	for (cnt=opts_len,op = opts;cnt;cnt--)
	{
		if (!(op->short_opt || op->long_opt))
		{
			vfbuf.offset = 0;
			if (!buf_start) {
				//Print only if any option found.
				_WRITESTROUT("Parameters:\n");
				buf_start=file->offset;
			}
			//Put indent 
			_FILLCHAR(' ',OPT_IDENT_SPACES);

			str = _get_oparg_name(op);
			if (str)
			{
				cl=strlen(str);
				// Add braces for optional arg or for required if only short opt was given
				if (op->has_arg==PO_OPTIONAL_ARG) 
				{
					_PUTCHAR( '[');
					_WRITE((const char*) str,cl);
					_PUTCHAR( ']');
				} else {
					_PUTCHAR( '<');
					_WRITE((const char*) str,cl);
					_PUTCHAR( '>');
				}
			}

			// Add description
			if (op->desc)
			{
				if ( vfbuf.offset < tpl ) {
					_FILLCHAR(' ',tpl-vfbuf.offset); // fill spaces up to col length
				} else {
					_WRITESTR(" -");
				}
				_WRITESTR(op->desc);
				_PUTCRLF;
			}
			// Print information about ranges.
			if ((ret=_print_range_desc(&vfbuf, op, max_column-tpl)<0)) goto _vfprint_opts_desc_end;

			// Print formated inforation to output file
			str = vfbuf.ptr; i=0;
			line = file->offset;
			while (str)
			{
				if (i) // Add ident for nonfirst line
					_FILLOUT(' ',line+tpl-file->offset); // fill spaces up to col length

				ptr = get_word_wrap(str,(unsigned) (max_column - (size_t) (file->offset-line)),&cl);
				if (!cl) {
					_PUTOUTCRLF;
					break;
				}
				_WRITEOUT(str,cl); //copy description

				// Add EOL characters;
				_PUTOUTCRLF;
				line=file->offset;
				if (!ptr) break;
				str = ptr; i++;
			}
		}

		op++;
	}

	// STEP3: Fill output buffer
  op = opts; buf_start=0;
  for (cnt=opts_len,op = opts;cnt;cnt--)
  {
    if (op->short_opt || op->long_opt)
    {
			vfbuf.offset = 0;

			if (!buf_start) {
				//Print only if any option found.
				_WRITESTROUT("Options:\n");
				buf_start=file->offset;
			}
      //Put indent 
			_FILLCHAR(' ',OPT_IDENT_SPACES);

      if (op->short_opt) {
        _PUTCHAR('-');
        _PUTCHAR(op->short_opt);
      }
      if (op->short_opt && op->long_opt) {
        _PUTCHAR(',');
        _PUTCHAR(' ');
      }
      if (op->long_opt)
      {
        cl=strlen(op->long_opt);
        _PUTCHAR('-'); _PUTCHAR('-');
        _WRITE(op->long_opt,cl);
      }

			str = _get_oparg_name(op);
      if (str)
      {
        if (!op->long_opt)_PUTCHAR(' '); //put space between short opt and val
        // Add braces for optional arg or for required if only short opt was given
        if (op->has_arg==PO_OPTIONAL_ARG)
        {
        	_PUTCHAR('[');
        }
        else if (!op->long_opt) _PUTCHAR('<');
        if (op->long_opt)_PUTCHAR('=');  // add "=" between long opt and value
        cl=strlen(str);
        _WRITE((const char*) str,cl);

        // Add braces for optional arg or for required if only short opt was given
        if (op->has_arg==PO_OPTIONAL_ARG)
		{
        	_PUTCHAR(']');
		}
        else if (!op->long_opt)
		{
        	_PUTCHAR('>');
		}
      }
  
      // Add description
      if (op->desc)
      {
        if ( vfbuf.offset < tol ) {
          _FILLCHAR(' ',tol-vfbuf.offset); // fill spaces up to col length
        } else {
          _WRITESTR(" -");
        }
        _WRITESTR(op->desc);
        _PUTCRLF;
      }
      // Print information about ranges.
			if ((ret=_print_range_desc(&vfbuf, op, max_column-tol)<0)) goto _vfprint_opts_desc_end;

			_PUTCHAR('\0'); //Put string terminator

      // Print formated information to output file
      str = vfbuf.ptr; i=0;
      line = file->offset;
      while (str)
      {
        if (i) // Add ident for nonfirst line
          _FILLOUT(' ',line+tol-file->offset); // fill spaces up to col length

        ptr = get_word_wrap(str,(unsigned) (max_column - (size_t) (file->offset-line)),&cl);
        if (!cl) {
          _PUTOUTCRLF;
          break;
        }
        _WRITEOUT(str,cl); //copy description
        
        // Add EOL characters;
        _PUTOUTCRLF;
        line=file->offset;
        if (!ptr) break;
        str = ptr; i++;
      }

    }

    op++;
  }

_vfprint_opts_desc_end:
  if (bytes_written) {
    *bytes_written=(file->offset>file->size) 
      ?  ((buf_start<file->size)? file->size-buf_start:0) 
      : file->offset-buf_start;
  }

  vfclose(&vfbuf);
  return ret;
#undef _WRITEOUT
#undef _WRITESTROUT
#undef _PUTOUT
#undef _PUTOUTCRLF
#undef _PUTCHAR
#undef _WRITE
#undef _WRITESTR
#undef _FILLCHAR
#undef _PUTCRLF

}
