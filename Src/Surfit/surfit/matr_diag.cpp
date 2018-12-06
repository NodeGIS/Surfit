
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

#include "surfit_ie.h"

#include "../sstuff/vec.h"
#include "../sstuff/bitvec.h"

#include "matr_diag.h"
#include "../sstuff/vec_alg.h"

#include <math.h>
#include <limits.h>

namespace surfit {

matr_diag::matr_diag(extvec * ival, 
		     size_t iN,
		     bitvec *& imask) {
	val = ival;
	N = iN;
	mask = imask;
	imask = NULL;
};

matr_diag::~matr_diag() {
	if (val)
		val->release();
	if (mask)
		mask->release();
};

REAL matr_diag::norm() const {
	return norm2(val->const_begin(), val->const_end());
};

size_t matr_diag::rows() const {
	return N;
};

size_t matr_diag::cols() const {
	return N;
};

REAL matr_diag::element_at(size_t i, size_t j, size_t * next_j) const 
{
	size_t _next_j = UINT_MAX;
	REAL res = REAL(0);

	if (i < j) {
		_next_j = UINT_MAX;
		goto exit;
	}

	if (i > j) {
		_next_j = i;
		goto exit;
	}

	if (i == j) {
		if (mask->get(j) == true)
			res = (*val)(j);
		_next_j = UINT_MAX;
		goto exit;
	}

exit:

	if (next_j) {
		*next_j = _next_j;
	}

	return res;	
};

REAL matr_diag::at(size_t i, size_t j, size_t * next_j) const 
{
	bool zero = mask->get(j);
	
	if (zero) {
		if ( next_j )
			*next_j = UINT_MAX;
		return REAL(0);
	}

	return element_at(i,j,next_j);
};

REAL matr_diag::mult_line(size_t J, extvec::const_iterator b_begin, extvec::const_iterator b_end) 
{
	if (mask->get(J))
		return (*val)(J) * *(b_begin+J);

	return REAL(0);
};

}; // namespace surfit;


