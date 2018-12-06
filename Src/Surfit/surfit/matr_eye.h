
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

#ifndef __surfit_matr_eye__
#define __surfit_matr_eye__

#include "matr.h"

namespace surfit {

class bitvec;

//! diagonal matrix with equal diagonal values
class SURFIT_EXPORT matr_eye : public matr {
public:
	//! constructor
	matr_eye(REAL ival, size_t iN, bitvec *& imask,
		 const bitvec * imask_solved, const bitvec * imask_undefined);

	//! A destructor
	virtual ~matr_eye();

	REAL element_at(size_t i, size_t j, size_t * next_j = NULL) const;
	REAL at(size_t i, size_t j, size_t * next_j = NULL) const;
	
	REAL mult_line(size_t J, extvec::const_iterator b_begin, extvec::const_iterator b_end);
	
	virtual size_t cols() const;
	virtual size_t rows() const;

	REAL norm() const;

protected:

	//! matrix size
	size_t N;

	//! value for main diagonal
	REAL val;

	//! mask for applying matrix
	bitvec * mask;

	//! another mask for applying matrix
	const bitvec * mask_solved;

	//! another mask for applying matrix
	const bitvec * mask_undefined;
};

}; // namespace surfit;

#endif


