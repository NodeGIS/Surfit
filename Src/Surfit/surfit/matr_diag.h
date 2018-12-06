
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

#ifndef __surfit_matr_diag__
#define __surfit_matr_diag__

#include "matr.h"
#include "../sstuff/vec.h"

namespace surfit {

class bitvec;

//! diagonal matrix
class SURFIT_EXPORT matr_diag : public matr {
public:
	//! constructor
	matr_diag(extvec * ival, size_t iN, bitvec *& imask);

	//! A destructor
	virtual ~matr_diag();

	REAL element_at(size_t i, size_t j, size_t * next_j = NULL) const;
	REAL at(size_t i, size_t j, size_t * next_j = NULL) const;
	
	REAL mult_line(size_t J, extvec::const_iterator b_begin, extvec::const_iterator b_end);
	
	virtual size_t cols() const;
	virtual size_t rows() const;

	REAL norm() const;

protected:

	//! matrix size
	size_t N;

	//! values for the main diagonal
	extvec * val;

	//! mask for applying matrix
	bitvec * mask;
	
};


}; // namespace surfit;

#endif


