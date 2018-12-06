
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

#include "f_trend.h"
#include "surf.h"
#include "grid_line.h"
#include "../sstuff/vec.h"
#include "matr.h"
#include "matrD1.h"
#include "matrD2.h"
#include "solvers.h"
#include "variables_tcl.h"
#include "../sstuff/bitvec.h"
#include "grid.h"
#include "curv.h"
#include "surf_internal.h"
#include "f_completer.h"
#include "grid_internal.h"
#include "f_fault.h"
#include "variables.h"

#include "grid_user.h"
#include "grid_line_user.h"

namespace surfit {

f_trend::f_trend(REAL iD1, REAL iD2, const d_surf * isrf) :
functional("f_trend", F_USUAL)
{
	srf = isrf;
	D1 = iD1;
	D2 = iD2;
	if (srf->getName()) {
		setNameF("f_trend %s", srf->getName());
	}
	tr_srf = NULL;

	trend_mask_solved = NULL;
	trend_mask_undefined = NULL;
	gfaults = NULL;
};

f_trend::~f_trend() {
	cleanup();
};

void f_trend::cleanup() {
	if (tr_srf)
		tr_srf->release_private();
	tr_srf = NULL;
	if (trend_mask_solved)
		trend_mask_solved->release();
	trend_mask_solved = NULL;
	if (trend_mask_undefined)
		trend_mask_undefined->release();
	trend_mask_undefined = NULL;
	if (gfaults)
		gfaults->release_private();
	gfaults = NULL;
};

int f_trend::this_get_data_count() const {
	return 1;
};

const data * f_trend::this_get_data(int pos) const {
	if (pos == 0)
		return srf;
	return NULL;
};

bool f_trend::minimize() {

	if (srf->getName())
		writelog(LOG_MESSAGE,"trend (%s) : processing...",srf->getName());
	else 
		writelog(LOG_MESSAGE,"trend : processing...");

	bool res = true;

	if (gfaults)
		gfaults->release();
	gfaults = NULL;
	grid_line * old_gfaults = NULL;
	size_t q, f_cnt = 0;
	bool have_faults = false;
	for (q = 0; q < functionals->size(); q++)
	{
		functional * f = (*functionals)[q];
		if (f->get_pos() >= get_pos())
			break;
		if (f->getType() != F_MODIFIER)
			continue;
		f_fault * flt = dynamic_cast<f_fault*>(f);
		if (flt == NULL)
			continue;

		const d_curv * fault_crv = flt->get_fault();
		if (fault_crv == NULL)
			continue;
		if (!have_faults) {
			writelog2(LOG_MESSAGE,"trend_faults: \"%s\"", fault_crv->getName());
			have_faults = true;
			f_cnt++;
		} else {
			if (f_cnt % 4 != 0) 
				log_printf(", \"%s\"", fault_crv->getName());
			else
				log_printf("\"%s\"", fault_crv->getName());
			f_cnt++;
			if (f_cnt % 4 == 0) {
				log_printf("\n");
				writelog2(LOG_MESSAGE,"              ");
			}
		}
		gfaults = curv_to_grid_line(gfaults, fault_crv, method_grid);
		if ((method_prev_grid != NULL) && (reproject_faults))
			old_gfaults = curv_to_grid_line(old_gfaults, fault_crv, method_prev_grid);
	}
	if (have_faults)
		log_printf("\n");

	if ( cond() ) {

		res = minimize_step() && res;

	} else { // use some optimizations

		if (reproject_faults && old_gfaults && method_prev_grid) 
		{
			writelog(LOG_MESSAGE,"solving near fault lines...");
			bitvec * saved_mask_solved = create_bitvec(method_mask_solved);
			
			method_mask_solved->init_true();
			size_t old_NN = method_prev_grid->getCountX();
			size_t old_MM = method_prev_grid->getCountY();
			size_t NN = method_grid->getCountX();
			size_t MM = method_grid->getCountY();
			size_t i, j;
			
			for (j = 0; j < old_MM; j++) {
				for (i = 0; i < old_NN; i++) {
					size_t pos = two2one(i, j, old_NN, old_MM);
					if (old_gfaults->check_for_node(pos) == false)
						continue;
					REAL x_from = method_prev_grid->x_from(i);
					REAL x_to = method_prev_grid->x_to(i);
					REAL y_from = method_prev_grid->y_from(j);
					REAL y_to = method_prev_grid->y_to(j);
					size_t i_from, i_to, j_from, j_to;
					i_from = MAX(0,method_grid->get_i(x_from));
					i_to = MIN(NN-1,method_grid->get_i(x_to));
					j_from = MAX(0,method_grid->get_j(y_from));
					j_to = MIN(MM-1,method_grid->get_j(y_to));
					size_t I,J;
					for (J = j_from; J <= j_to; J++) {
						for (I = i_from; I <= i_to; I++) {
							size_t POS = two2one(I, J, NN, MM);
							if (saved_mask_solved->get(POS) == true)
								continue;
							if (method_mask_undefined->get(POS) == true)
								continue;
							method_mask_solved->set_false(POS);
						}
					}
				}
			}
			
			res = minimize_step();
			
			method_mask_solved->copy(saved_mask_solved);
			if (saved_mask_solved) {
				saved_mask_solved->release();
				saved_mask_solved = NULL;
			}
			
			old_gfaults->release();
			old_gfaults = NULL;
		}

		if (reproject_undef_areas) {
			if (method_prev_grid != NULL) {
				writelog(LOG_MESSAGE,"solving near undef areas...");
											
				grid_line * undef_grd_line = trace_undef_grd_line(method_mask_undefined, method_grid->getCountX());
				if (undef_grd_line) {
					
					bitvec * saved_mask_solved = create_bitvec(method_mask_solved);
					int size = method_mask_solved->size();
					int i;
					
					for (i = 0; i < size; i++) {
						if (!undef_grd_line->check_for_node(i)) 
							method_mask_solved->set_true(i);
					};
					
					//res = minimize_step() && res;
					minimize_step();
					
					undef_grd_line->release();
					method_mask_solved->copy(saved_mask_solved);
					if (saved_mask_solved) 
						saved_mask_solved->release();
					
				}

			}
		}

		size_t matrix_size = method_basis_cntX*method_basis_cntY;

		shortvec * flood_areas = create_shortvec(matrix_size);
		int flood_areas_cnt = 0;

		fill_all_areas(flood_areas, method_grid, gfaults, flood_areas_cnt, method_mask_undefined);

		if ((flood_areas_cnt > 1) && (process_isolated_areas)) {
			
			writelog(LOG_MESSAGE,"trend : flood_areas_cnt = %d", flood_areas_cnt);
			writelog(LOG_MESSAGE,"trend : processing each isolated area (%d)...", flood_areas_cnt);
			
			bitvec * saved_mask_solved = create_bitvec(method_mask_solved);
			bitvec * saved_mask_undefined = create_bitvec(method_mask_undefined);
			
			int color;
			size_t f_size = flood_areas->size();
			
			bool undef, solved;
			
			size_t pos = 0;
			int exists = 0;
			
			for (color = 1; color <= flood_areas_cnt; color++) {

				writelog(LOG_MESSAGE, "trend : proccessing isolated trend area N%d",color);
				
				if (color > 1) {
					method_mask_undefined->copy(saved_mask_undefined);
					method_mask_solved->copy(saved_mask_solved);
				}
				
				for (pos = 0; pos < f_size; pos++) {
					if ( (*flood_areas)(pos) != color ) {
						method_mask_undefined->set_true(pos);
						method_mask_solved->set_false(pos);
					}
				}
				
				res = minimize_step();
				
				if (res) {
					for (pos = 0; pos < f_size; pos++) {
						if ( (*flood_areas)(pos) == color ) {
							
							undef = method_mask_undefined->get(pos);
							if (undef)
								saved_mask_undefined->set_true(pos);
							else 
								saved_mask_undefined->set_false(pos);
							
							solved = method_mask_solved->get(pos);	
							if (solved)
								saved_mask_solved->set_true(pos);
							else
								saved_mask_solved->set_false(pos);
							
						}
					}
				} else {
					/*
					for (pos = 0; pos < f_size; pos++) {
						if ( (*flood_areas)(pos) == color ) {
							saved_mask_undefined->set_true(pos);
						}
					}
					*/
				}	
				
			};
						
			method_mask_solved->copy(saved_mask_solved);
			method_mask_undefined->copy(saved_mask_undefined);
			
			if (saved_mask_solved)
				saved_mask_solved->release();
			if (saved_mask_undefined)
				saved_mask_undefined->release();
			
		}

		if (flood_areas)
			flood_areas->release();

		writelog(LOG_MESSAGE, "trend : proccessing whole trend area...");
		
		res = minimize_step(); // && res;

	}

	if (old_gfaults)
		old_gfaults->release();

	return true;
};

bool f_trend::make_matrix_and_vector(matr *& matrix, extvec *& v, bitvec * mask_solved, bitvec * mask_undefined) {
	
	size_t matrix_size = method_basis_cntX*method_basis_cntY;
	size_t NN = method_grid->getCountX();
	size_t MM = method_grid->getCountY();

	size_t aux_X_from, aux_X_to;
	size_t aux_Y_from, aux_Y_to;
	get_tr_srf(aux_X_from, aux_X_to, aux_Y_from, aux_Y_to);
	if (tr_srf == NULL)
		return false;

	size_t nn = aux_X_to-aux_X_from+1;
	size_t mm = aux_Y_to-aux_Y_from+1;
	
	trend_mask_solved = create_bitvec(mask_solved);
	trend_mask_undefined = create_bitvec(mask_undefined);

	size_t i,j;
	for (j = aux_Y_from; j <= aux_Y_to; j++) {
		for (i = aux_X_from; i <= aux_X_to; i++) {
			REAL val = tr_srf->getValueIJ(i-aux_X_from,j-aux_Y_from);
			if (val == tr_srf->undef_value) {
				size_t pos = i + j*NN;
				trend_mask_undefined->set_true(pos);
				trend_mask_solved->set_false(pos);

				if ((D1 > 0) || (D2 > 0)) {
					
					if (j >= aux_Y_from+1) {
						pos = i + (j-1)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
					if (i+1 <= aux_X_to) {
						pos = i+1 + j*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
					if (i >= aux_X_from+1) {
						pos = i-1 + j*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
					if (j+1 <= aux_Y_to) {
						pos = i + (j+1)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
				}

				if (D2 > 0) {

					if ((i >= aux_X_from+1) && (j >= aux_Y_from+1)) {
						pos = i-1 + (j-1)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}

					if ((i+1 <= aux_X_to) && (j >= aux_Y_from+1)) {
						pos = i+1 + (j-1)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}

					if ((i >= aux_X_from+1) && (j+1 <= aux_Y_to)) {
						pos = i-1 + (j+1)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
					if ((i+1 <= aux_X_to) && (j+1 <= aux_Y_to)) {
						pos = i+1 + (j+1)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}

					if (j >= aux_Y_from+2) {
						pos = i + (j-2)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
					if (i+2 <= aux_X_to) {
						pos = i+2 + j*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
					if (i >= aux_X_from+2) {
						pos = i-2 + j*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
					
					if (j+2 <= aux_Y_to) {
						pos = i + (j+2)*NN;
						trend_mask_undefined->set_true(pos);
						trend_mask_solved->set_false(pos);
					}
										

				}

				
			}
		}
	}

	matr * T = NULL;
	size_t points;

	if ( (nn == NN)  && (mm == MM) ) {
		matrD1 * oD1 = NULL;
		matrD2 * oD2 = NULL;
		
		if (D1 > 0)
			oD1 = new matrD1(matrix_size, NN, 
					 method_grid->stepX, 
					 method_grid->stepY,
					 trend_mask_solved, trend_mask_undefined, 
					 gfaults); 
		
		if (D2 > 0)
			oD2 = new matrD2(matrix_size, NN, 
					 method_grid->stepX, 
					 method_grid->stepY,
					 trend_mask_solved, trend_mask_undefined, 
					 gfaults); 

		if ((oD1 != NULL) && (oD2 == NULL)) 
			T = oD1;
		if ((oD2 != NULL) && (oD1 == NULL))
			T = oD2;
		
		if (T == NULL)
			T = new matr_sum(D1, oD1, 
					 D2, oD2);

		v = create_extvec(matrix_size);

		points = calcVecV(matrix_size, method_X, T, v, NN, MM, 
				  trend_mask_solved,
				  trend_mask_undefined, 
				  UINT_MAX, UINT_MAX,
				  UINT_MAX, UINT_MAX,
				  tr_srf);
	} else {
		
		matrD1_rect * oD1 = NULL;
		matrD2_rect * oD2 = NULL;
		
		if (D1 > 0)
			oD1 = new matrD1_rect(matrix_size, NN, 
					      method_grid->stepX, 
					      method_grid->stepY,
					      aux_X_from, aux_X_to,
					      aux_Y_from, aux_Y_to,
					      trend_mask_solved, trend_mask_undefined, 
					      gfaults); 
		
		if (D2 > 0)
			oD2 = new matrD2_rect(matrix_size, NN, 
					      method_grid->stepX, 
					      method_grid->stepY,
					      aux_X_from, aux_X_to,
					      aux_Y_from, aux_Y_to,
					      trend_mask_solved, trend_mask_undefined, 
					      gfaults); 
		
		
		if ((oD1 != NULL) && (oD2 == NULL)) 
			T = dynamic_cast<matrD1*>(oD1);
		if ((oD2 != NULL) && (oD1 == NULL))
			T = dynamic_cast<matrD2*>(oD2);
		
		if (T == NULL)
			T = new matr_rect_sum(aux_X_from, aux_X_to,
					      aux_Y_from, aux_Y_to, NN,
					      D1, oD1, 
					      D2, oD2);

		v = create_extvec(matrix_size);

		points = calcVecV(matrix_size, method_X, T, v, NN, MM, 
				  trend_mask_solved,
				  trend_mask_undefined, 
				  aux_X_from, aux_X_to,
				  aux_Y_from, aux_Y_to,
				  tr_srf);
	}

	trend_mask_solved->release();
	trend_mask_solved = NULL;

	matrix = T;

	bool solvable = completer_solvable(points, D1, D2);

	solvable = wrap_sums(matrix, v, mask_solved, mask_undefined) || solvable;
	return solvable;
};

void f_trend::mark_solved_and_undefined(bitvec * mask_solved, bitvec * mask_undefined, bool i_am_cond) 
{
	size_t NN = method_grid->getCountX();

	size_t aux_X_from, aux_X_to;
	size_t aux_Y_from, aux_Y_to;
	_grid_intersect1(method_grid, srf->grd, aux_X_from, aux_X_to, aux_Y_from, aux_Y_to);
	
	size_t i,j,pos;
	for (j = aux_Y_from; j <= aux_Y_to; j++) {
		for (i = aux_X_from; i <= aux_X_to; i++) {
			pos = i + j*NN;
			if (trend_mask_undefined) {
				if (!mask_undefined->get(pos) && !trend_mask_undefined->get(pos)) {
					REAL val = tr_srf->getValueIJ(i-aux_X_from, j-aux_Y_from);
					mask_solved->set_true(pos);
				}
			} else {
				if (!mask_undefined->get(pos))  {
					REAL val = tr_srf->getValueIJ(i-aux_X_from, j-aux_Y_from);
					mask_solved->set_true(pos);
				}
			}
		}
	}

	if (trend_mask_undefined)
		trend_mask_undefined->release();
	trend_mask_undefined = NULL;
	
	mark_sums(mask_solved, mask_undefined);

};

bool f_trend::minimize_step() {
	
	matr * A = NULL;
	extvec * b = NULL;
	bool solvable = make_matrix_and_vector(A,b,method_mask_solved,method_mask_undefined);

	size_t matrix_size = method_basis_cntX*method_basis_cntY;

	if ( !cond() ) {
		if (solvable == false) {
			delete A;
			if (b)
				b->release();
			return false;
		}
		
		method_X->resize(b->size());
		solve(A,b,method_X);
		method_X->resize(matrix_size);
		
		delete A;
		if (b)
			b->release();
		
		return true;
	} else {
		return solve_with_penalties(this, A, b, method_X);
	}
	return false;
};

bool f_trend::solvable_without_cond(const bitvec * mask_solved,
				    const bitvec * mask_undefined,
				    const extvec * X)
{

	size_t NN = method_grid->getCountX();

	size_t aux_X_from, aux_X_to;
	size_t aux_Y_from, aux_Y_to;
	_grid_intersect1(method_grid, srf->grd, aux_X_from, aux_X_to, aux_Y_from, aux_Y_to);
	
	size_t i,j,pos;
	size_t cnt = 0;
	for (j = aux_Y_from; j <= aux_Y_to; j++) {
		for (i = aux_X_from; i <= aux_X_to; i++) {
			pos = i + j*NN;
			if (mask_solved->get(pos))
				cnt++;
			if (D2 == 0) {
				if (cnt == 1) {
					goto sss;
				}
			}
			if (cnt == 3) {
				goto sss;
			}
		}
	}
	
	return false;

sss:

	bool res = true;
	
	return res;

};

void f_trend::get_tr_srf(size_t & i_from, size_t & i_to, size_t & j_from, size_t & j_to) {
	
	_grid_intersect1(method_grid, srf->grd, i_from, i_to, j_from, j_to);
	d_grid * aux_grid = _create_sub_grid(method_grid, i_from, i_to, j_from, j_to);

	if (tr_srf == NULL)
		tr_srf = _surf_project(srf, aux_grid);
	else {
		if (tr_srf->grd->operator==(aux_grid) == false) {
			tr_srf->release();
			tr_srf = _surf_project(srf, aux_grid);
		}
	}
	
	if (aux_grid)
		aux_grid->release();
};

void f_trend::drop_private_data() {
	if (tr_srf)
		tr_srf->release_private();
	tr_srf = NULL;
};

}; // namespace surfit;

