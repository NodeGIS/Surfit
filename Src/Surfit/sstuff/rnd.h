
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

#ifndef __rnd_included__
#define __rnd_included__

/*! \file
    \brief declaration of functions for working with random numbers
*/
	
namespace surfit {

//! randomization function 
SSTUFF_EXPORT
void randomize();

//! returns random number with normal distribution N(0,std)
SSTUFF_EXPORT
REAL norm_rand(REAL std);

}; // namespace surfit;

#endif

