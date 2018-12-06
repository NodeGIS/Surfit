
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

/*! \file grid_internal.cpp
    implementation of "internal" functions for grid class
*/

#include "surfit_ie.h"

#include "../sstuff/fileio.h"
#include "../sstuff/datafile.h"
#include "../sstuff/vec.h"
#include "../sstuff/vec_alg.h"

#include <math.h>
#include "grid.h"
#include "grid_internal.h"
#include "points.h"
#include "pnts_internal.h"
#include "surf.h"

#include "grid_user.h"
#include <algorithm>

namespace surfit {

bool _grid_check() {
	if (surfit_grid) 
		return true;
	return false;
};

bool _grid_save_df(d_grid * grd, datafile * df) {
	bool res = true;
	bool op;
	
	op = grd->writeTags(df);				res = (op && res);
	
	return res;
};

bool _grid_save(d_grid * grd, const char * filename) {
	if (!grd) {
		writelog(LOG_WARNING, "NULL pointer to grid.");
		return false;
	};
	datafile *df = new datafile(filename, DF_MODE_WRITE); // write
	if (!df->condition())
		return false;
	
	bool res = _grid_save_df(grd, df);
	
	bool op = df->writeEof();				res = ( op && res );
	
	delete df;
	return res;
};

d_grid * _grid_get(REAL startX, REAL endX, REAL stepX,
                   REAL startY, REAL endY, REAL stepY,
                   const char * name) {
	
	return create_grid(startX, endX, stepX,
			   startY, endY, stepY,
			   name);
	
};

d_grid * _grid_get_for_pnts(d_points * pnts, int Xnodes, int Ynodes, const char * name) {
	
	if (pnts->size() == 0) {
		writelog(LOG_WARNING,"grid_get_for_pnts : empty points");
		return NULL;
	}
	
	REAL minx = pnts->minx();
	REAL maxx = pnts->maxx();
	
	REAL len_x = maxx-minx;
	if (len_x == REAL(0))
		len_x = REAL(1);
	
	REAL miny = pnts->miny();
	REAL maxy = pnts->maxy();
	
	REAL len_y = maxy-miny;
	if (len_y == REAL(0))
		len_y = REAL(1);
	
	d_grid * res = create_grid(minx, maxx, (maxx-minx)/(Xnodes - 1),
				   miny, maxy, (maxy-miny)/(Ynodes - 1),
				   name);
	
	return res;
};

d_grid * _grid_get_for_pnts_step(d_points * pnts, REAL stepX, REAL stepY, const char * name) {

	if (pnts->size() == 0) {
		writelog(LOG_WARNING,"grid_get_for_pnts_step : empty pnts");
		return NULL;
	}

	REAL minx = pnts->minx();
	REAL maxx = pnts->maxx();

	REAL len_x = maxx-minx;
	if (len_x == REAL(0))
		len_x = REAL(1);

	REAL miny = pnts->miny();
	REAL maxy = pnts->maxy();
    
	REAL len_y = maxy-miny;
	if (len_y == REAL(0))
		len_y = REAL(1);

	d_grid * res = create_grid(minx, maxx, stepX,
                                   miny, maxy, stepY,
                                   name);

	return res;
};

d_grid * _grid_get_for_surf(d_surf * srf, int Xnodes, int Ynodes, const char * name) {
	
	REAL minx = srf->getMinX();
	REAL maxx = srf->getMaxX();
	REAL miny = srf->getMinY();
	REAL maxy = srf->getMaxY();

	REAL len_x = maxx-minx;
	if (len_x == REAL(0))
		len_x = REAL(1);

	REAL len_y = maxy-miny;
	if (len_y == REAL(0))
		len_y = REAL(1);

	d_grid * res = create_grid(minx, maxx, (maxx-minx)/(Xnodes - 1),
                                   miny, maxy, (maxy-miny)/(Ynodes - 1),
                                   name);

	return res;
};

d_grid * _grid_get_for_surf_step(d_surf * srf, REAL stepX, REAL stepY, const char * name) {

	REAL minx = srf->getMinX();
	REAL maxx = srf->getMaxX();
	REAL miny = srf->getMinY();
	REAL maxy = srf->getMaxY();

	REAL len_x = maxx-minx;
	if (len_x == REAL(0))
		len_x = REAL(1);

	REAL len_y = maxy-miny;
	if (len_y == REAL(0))
		len_y = REAL(1);

	d_grid * res = create_grid(minx, maxx, stepX,
                                   miny, maxy, stepY,
                                   name);

	return res;

};

bool _grid_load_df_tag_readed(datafile * df, d_grid *& geom) {
	
	REAL startX = 0;
	REAL endX = 0;
	REAL stepX = 0;
	REAL startY = 0;
	REAL endY = 0;
	REAL stepY = 0;
	
	REAL angle = 0;
	REAL pi = REAL(3.1415926535);
	REAL cos_val;
	REAL sin_val;
	REAL len;
	
	char * name = NULL;
	
	d_grid * res = NULL;
	
	if (!df->readWord()) goto exit;
	
	while ( !df->isWord("endtag" ) ) {
		
		if ( df->isWord("char") ) {
			// read char name
			if (!df->readWord()) goto exit;
			if ( df->isWord("name") ) {
				if ( !df->readWord() ) goto exit;
				
				name = (char *)malloc(strlen(df->getWord()) + 1);
				strcpy(name, df->getWord());
				goto cont;
			}
		}
		
		if ( df->isWord( REAL_NAME ) ) {
			
			if (!df->readWord()) goto exit;
			
			if ( df->isWord("startX") ) {
				if ( !df->readReal(startX) ) goto exit;
				goto cont;
			}
			if ( df->isWord("endX") ) {
				if ( !df->readReal(endX) ) goto exit;
				goto cont;
			}
			if ( df->isWord("stepX") ) {
				if ( !df->readReal(stepX) ) goto exit;
				goto cont;
			}
			if ( df->isWord("startY") ) {
				if ( !df->readReal(startY) ) goto exit;
				goto cont;
			}
			if ( df->isWord("endY") ) {
				if ( !df->readReal(endY) ) goto exit;
				goto cont;
			}
			if ( df->isWord("stepY") ) {
				if ( !df->readReal(stepY) ) goto exit;
				goto cont;
			}
			if ( df->isWord("angle") ) {
				if ( !df->readReal(angle) ) goto exit;
				goto cont;
			}
			if (!df->skipReal(false)) goto exit;
			goto cont;
		}
		
		if ( !df->skip(false) ) goto exit;
		goto cont;
cont:
		if ( !df->readWord() ) goto exit;
		continue;
	}
	
	
	angle = angle/REAL(180)*pi;
	len = sqrt((endX-startX)*(endX-startX) + (endY-startY)*(endY-startY));
	angle = angle + acos((endX-startX)/len);
	cos_val = cos(angle);
	sin_val = sin(angle);
	
	endX = startX + len*cos_val;
	endY = startY + len*sin_val;
	
	res = create_grid(startX, endX, stepX,
			  startY, endY, stepY,
			  name);
	
	if (geom)
		geom->release();
	geom = res;
	
	return true;
	
exit:
	
	free(name);
	return false;
};

d_grid * _grid_load_df(datafile * df, const char * grid_name) {
	
	d_grid * grd = NULL;
	
	bool loaded = false;
	
	while (!loaded) {
		
		if (df->findTag("grid")) {
			
			df->skipTagName();
			
			if (!_grid_load_df_tag_readed(df, grd)) {
				if (grd)
					grd->release();
				return NULL;
			};
			
			if (!grd) {
				return NULL;
			}
			
			if (!grid_name) {
				return grd;
			}
			
			if (!grd->getName()) {
				if (!grid_name)
					return grd;
				if (strlen(grid_name) == 0)
					return grd;
				
				if (grd)
					grd->release();
				grd = NULL;
				continue;
			};
			
			if ( strcmp(grid_name,grd->getName()) == 0 ) {
				return grd;
			}
			
			if (grd)
				grd->release();
			grd = NULL;
			
			
		} else {
			if (!grid_name)
				writelog(LOG_ERROR, "grid_load : this file have no grid");
			else
				writelog(LOG_ERROR, "grid_load : this file have no grid with name %s", grid_name);
			if (grd)
				grd->release();
			return NULL;
		}
		
		
	};
	
	if (grd)
		grd->release();

	return NULL;
};

d_grid * _grid_load(const char * filename, const char * gridname) {

	datafile *df = new datafile(filename, DF_MODE_READ);
	
	if (!df->condition()) {
		delete df;
		return NULL;
	}

	d_grid * grd = _grid_load_df(df, gridname);
		
	delete df;
	return grd;

};

d_grid * _grid_from_surf(d_surf * srf, const char * name) {
	if (!srf)
		return NULL;
	d_grid * res = NULL;
	if (name)
		res = create_grid(srf->grd, name);
	else
		res = create_grid(srf->grd, srf->grd->getName());
	return res;
};

void _set_surfit_grid(d_grid * grd) {
	if (surfit_grid)
		surfit_grid->release();
	surfit_grid = grd;
};

d_grid * _get_surfit_grid() {
	return surfit_grid;
};

void _grid_intersect1(const d_grid * grd1, const d_grid * grd2,
		      size_t & x_from, size_t & x_to,
		      size_t & y_from, size_t & y_to) 
{
	REAL x_min2 = grd2->x_from(0);
	REAL x_max2 = grd2->x_to(grd2->getCountX()-1) - (grd2->stepX*1e-6);

	REAL y_min2 = grd2->y_from(0);
	REAL y_max2 = grd2->y_to(grd2->getCountY()-1) - (grd2->stepY*1e-6);

	x_from = grd1->get_i(x_min2);
	x_to = grd1->get_i(x_max2);
	y_from = grd1->get_j(y_min2);
	y_to = grd1->get_j(y_max2);

	x_from = MAX(0, x_from);
	x_from = MIN(grd1->getCountX()-1, x_from);
	
	x_to = MAX(0, x_to);
	x_to = MIN(grd1->getCountX()-1, x_to);

	y_from = MAX(0, y_from);
	y_from = MIN(grd1->getCountY()-1, y_from);

	y_to = MAX(0, y_to);
	y_to = MIN(grd1->getCountY()-1, y_to);

};

d_grid * _create_sub_grid(const d_grid * grd, int x_from, int x_to, int y_from, int y_to) {
	REAL X_from = grd->startX + x_from*grd->stepX;
	REAL X_to = grd->startX + x_to*grd->stepX;
	REAL Y_from = grd->startY + y_from*grd->stepY;
	REAL Y_to = grd->startY + y_to*grd->stepY;

	d_grid * res = create_grid(X_from, X_to, grd->stepX,
		                   Y_from, Y_to, grd->stepY,
			           grd->getName());

	return res;

};

d_grid * _grid_get_best( std::vector<d_grid *> * grids )
{
	size_t i;
	REAL stepX = FLT_MAX;
	REAL stepY = FLT_MAX;
	REAL startX = FLT_MAX;
	REAL startY = FLT_MAX;
	REAL endX = -FLT_MAX;
	REAL endY = -FLT_MAX;
	for (i = 0; i < grids->size(); i++)
	{
		d_grid * grd = (*grids)[i];
		stepX = MIN(grd->stepX, stepX);
		stepY = MIN(grd->stepY, stepY);
		startX = MIN(grd->startX, startX);
		startY = MIN(grd->startY, startY);
		endX = MAX(grd->endX, endX);
		endY = MAX(grd->endY, endY);
	}
	d_grid * res = create_grid(startX, endX, stepX,
				   startY, endY, stepY);
	return res;
};

}; // namespace surfit;

