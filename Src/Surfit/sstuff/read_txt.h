
/*------------------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2002-2006 by M. V. Dmitrievsky and V. N. Kutrunov
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: surfit.sourceforge.net
 *----------------------------------------------------------------------------*/

#ifndef __surfit_read_txt_included__
#define __surfit_read_txt_included__

/*! \file
    \brief declaration of functions for reading/writing operations with text files
*/

namespace surfit {

class vec;
class strvec;

//! reads one column from text file
SSTUFF_EXPORT
bool one_columns_read(const char * filename, 
                      int col1, int skip_lines,
                      const char * mask, int grow_by,
                      vec *& vcol1, int read_lines = -1);

//! writes one column to text file
SSTUFF_EXPORT
bool one_columns_write(const char * filename, const char * mask,
		       const vec * vcol1);

//! reads two columns from text file
SSTUFF_EXPORT
bool two_columns_read(const char * filename, 
                        int col1, int col2, int skip_lines,
                        const char * mask, int grow_by,
                        vec *& vcol1, vec *& vcol2,
			int read_lines = -1);

//! writes two colums to text file
SSTUFF_EXPORT
bool two_columns_write(const char * filename, const char * mask,
		       const vec * vcol1, const vec * vcol2);

//! reads three columns from text file
SSTUFF_EXPORT
bool three_columns_read(const char * filename, 
                        int col1, int col2, int col3, int skip_lines,
                        const char * mask, int grow_by,
                        vec *& vcol1, vec *& vcol2, vec *& vcol3,
			int read_lines = -1);

//! reads four (!) columns from text file, where three of them a numerical and fourth is text
SSTUFF_EXPORT
bool three_columns_read_with_names(const char * filename, 
				   int col1, int col2, int col3, int col4,
				   int skip_lines,
				   const char * mask, int grow_by,
				   vec *& vcol1, vec *& vcol2, vec *& vcol3,
				   strvec *& names,
				   int read_lines = -1);

//! writes three columns to file
SSTUFF_EXPORT
bool three_columns_write(const char * filename, const char * mask,
			 const vec * vcol1, const vec * vcol2, const vec * vcol3);

//! reads four columns from text file
SSTUFF_EXPORT
bool four_columns_read(const char * filename, 
                       int columns, 
                       int col1, int col2, int col3, int col4,
                       const char * mask, int grow_by,
                       vec *& vcol1, vec *& vcol2,
                       vec *& vcol3, vec *& vcol4);

//! writes four columns to text file
SSTUFF_EXPORT
bool four_columns_write(const char * filename, const char * mask,
			const vec * vcol1, const vec * vcol2, 
			const vec * vcol3, const vec * vcol4);

//! converts string to number
SSTUFF_EXPORT
REAL ator(char * text);

}; // namespace surfit;

#endif

