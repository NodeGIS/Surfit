
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

#include "../sstuff/fileio.h"

#include "f_ineq.h"
#include "../sstuff/bitvec.h"
#include "../sstuff/vec.h"
#include "matr_diag.h"
#include "variables_tcl.h"

#include "grid_user.h"

namespace surfit {

f_ineq::f_ineq(REAL ivalue, bool ileq, REAL imult) :
functional("f_ineq", F_CONDITION)
{
	leq = ileq;
	value = ivalue;
	mult = imult;
};

f_ineq::~f_ineq() {};

int f_ineq::this_get_data_count() const {
	return 0;
};

const data * f_ineq::this_get_data(int pos) const {
	return NULL;
};

bool f_ineq::minimize() {

	return false;

};

bool f_ineq::make_matrix_and_vector(matr *& matrix, extvec *& v, bitvec * mask_solved, bitvec * mask_undefined) 
{
	size_t points = 0;
	size_t i;
	
	size_t matrix_size = method_basis_cntX*method_basis_cntY;
	v = create_extvec(matrix_size);

	if (leq)
		writelog(LOG_MESSAGE,"inequality leq %g", value);
	else 
		writelog(LOG_MESSAGE,"inequality geq %g", value);
		
	bitvec * mask = create_bitvec(matrix_size);
	mask->init_false();

	extvec * diag = create_extvec(matrix_size);

	for (i = 0; i < matrix_size; i++) {
		
		if (mask_solved->get(i))
			continue;
		if (mask_undefined->get(i))
			continue;
		
		REAL x_value = (*method_X)(i);
		
		REAL dist = fabs(value - x_value);
		
		if (leq) {
			if (x_value > value) {
				mask->set_true(i);
				(*v)(i) = value*mult*(dist + 1);
				(*diag)(i) = mult*(dist + 1);
				points++;
			}
		} else {
			if (x_value < value) {
				mask->set_true(i);
				(*v)(i) = value*mult*(dist + 1);
				(*diag)(i) = mult*(dist + 1);
				points++;
			}
		}

		
	}

	matr_diag * T = new matr_diag(diag, matrix_size, mask);
	matrix = T;

	if (points == 0) {
		delete T;
		T = NULL;
		matrix = NULL;
		if (v)
			v->release();
		v = NULL;
	}

	bool solvable = true;

	solvable = wrap_sums(matrix, v, mask_solved, mask_undefined) || solvable;
	return solvable;
};

void f_ineq::mark_solved_and_undefined(bitvec * mask_solved, bitvec * mask_undefined, bool i_am_cond) 
{
	set_solved(mask_solved, mask_undefined);
	mark_sums(mask_solved, mask_undefined);
};

bool f_ineq::solvable_without_cond(const bitvec * mask_solved,
			           const bitvec * mask_undefined,
			           const extvec * X)
{
	return true;
	/*
	size_t matrix_size = method_basis_cntX*method_basis_cntY;

	size_t i;
	for (i = 0; i < matrix_size; i++) {
		
		if (mask_solved->get(i))
			continue;
		if (mask_undefined->get(i))
			continue;

		return false;

	}

	return true;
	*/
};


}; // namespace surfit;

