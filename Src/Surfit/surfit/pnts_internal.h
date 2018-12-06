
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

#ifndef __surfit__pnts_internal__
#define __surfit__pnts_internal__

#include <vector>

namespace surfit {

class vec;
class d_grid;
class datafile;
class d_points;
class sub_points;

////////////////////////////////////////////////
////////////////////////////////////////////////
//                points io                   //
////////////////////////////////////////////////
////////////////////////////////////////////////

///////////////
// load

/*! \brief reads points from formatted text file
    \return pointer to \ref d_points class
    \param filename filename
    \param pntsname pntsname
    \param col1 column number in text file for X coordinates
    \param col2 column number in text file for Y coordinates
    \param col3 column number in text file for Z coordinates
    \param col4 column number in text file for names (0 - if no names present)
    \param mask mask for scanf
    \param grow_by vector's grow factor
    \param skip_lines number of lines to skip header
*/
SURFIT_EXPORT
d_points * _pnts_read(const char * filename, const char * pntsname, 
		      int col1, int col2, int col3, int col4,
		      const char * mask, int skip_lines, int grow_by);

//! reads \ref d_points from surfit \ref datafile
SURFIT_EXPORT
d_points * _pnts_load(const char * filename, const char * pntsname);

//! reads \ref d_points from \ref datafile
SURFIT_EXPORT
d_points * _pnts_load_df(datafile * df, const char * pntsname);

//////////////
// save

/*! \brief saves \ref d_points to formatted text file
    \param pnts pointer to \ref d_points
    \param filename filename
    \param mask mask for printf
*/
SURFIT_EXPORT
bool _pnts_write(const d_points * pnts, const char * filename, const char * mask);

/*! \brief saves \ref d_points to surfit \ref datafile
    \param filename filename
    \param pnts pointer to \ref d_points
*/
SURFIT_EXPORT
bool _pnts_save(const d_points * pnts, const char * filename);

//! writes \ref d_points tags to \ref datafile
SURFIT_EXPORT
bool _pnts_save_df(const d_points * pnts, datafile * df);

//////////////
// stuff

//! prints some info about \ref d_points
SURFIT_EXPORT
void _pnts_info(const d_points * pnts);

//! makes one sub_points from all given points
SURFIT_EXPORT
void prepare_scattered_points(const d_points * pnts, 
                              std::vector<sub_points *> *& proc_sub_tsks);

//! frees memory
SURFIT_EXPORT
void free_scattered_points(std::vector<sub_points *> * proc_sub_tsks);

//! binds points to grid (calculates new sub_points)
SURFIT_EXPORT
void bind_points_to_grid(d_grid *& old_grid, 
			 const d_points * pnts,
			 std::vector<sub_points *> *& old_pnts,
			 d_grid *& grd);

//! sort function for sub_points
SURFIT_EXPORT
bool ptr_sub_points_less(sub_points * it1, sub_points * it2);

//! adds \ref d_points to collection
SURFIT_EXPORT
void _surfit_pnts_add(d_points * pnts);

//! converts \ref d_points to \ref d_cntr and adds resulting \ref d_cntr to collection
SURFIT_EXPORT
bool _pnts_to_cntrs(const d_points * pnts);

}; // namespace surfit;

#endif

