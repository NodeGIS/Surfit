
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

#ifndef __surfit__grid__
#define __surfit__grid__

#include <math.h>
#include <limits.h>

namespace surfit {

class datafile;
class d_grid;

//! constructor for \ref d_grid
SURFIT_EXPORT
d_grid * create_grid(REAL startX, REAL endX, REAL stepX,
	       REAL startY, REAL endY, REAL stepY,
	       const char * iname = NULL);

//! constructor for \ref d_grid
SURFIT_EXPORT
d_grid * create_grid(const d_grid * igrid, const char * iname = NULL);

/*! \class d_grid
    \brief class for equidistant rectangular 2d-grid
*/
class SURFIT_EXPORT d_grid {
protected:

	/*! constructor
	    \param startX X-coordinate of the first node
	    \param endX X-coordinate of the last node
	    \param stepX X-distance between first and second nodes
	    \param startY Y-coordinate of the first node
	    \param endY Y-coordinate of the last nodes
	    \param stepY Y-distance between first and second nodes
	    \param iname grid name
	*/
	d_grid(REAL startX, REAL endX, REAL stepX,
	       REAL startY, REAL endY, REAL stepY,
	       const char * iname = NULL);

	//! copy constructor
	d_grid(const d_grid * igrid, const char * iname = NULL);

	//! destructor
        virtual ~d_grid();

public:

	friend SURFIT_EXPORT
	d_grid * create_grid(REAL startX, REAL endX, REAL stepX,
			     REAL startY, REAL endY, REAL stepY,
			     const char * iname);

	friend SURFIT_EXPORT
	d_grid * create_grid(const d_grid * igrid, const char * iname);

	//! destructor 
	virtual void release();

	//! calculates the amount of nodes in X direction
	size_t getCountX() const;

	//! sets the amount of nodes in X direction
	bool setCountX(size_t countX);

	//! calculates the amount of nodes in Y direction
	size_t getCountY() const;

	//! sets the amount of nodes in Y direction
	bool setCountY(size_t countY);

	//! returns coordinates of (i,j) node
	void getCoordNode(size_t i, size_t j, REAL & x, REAL & y) const;

	//! returns X-coordinate of (i,j) node
	REAL getCoordNodeX(size_t i) const { 
		if (i == UINT_MAX)
			return startX - stepX;
		return startX + (i)*stepX; 
	};

	//! returns Y-coordinate of (i,j) node
	REAL getCoordNodeY(size_t j) const { 
		if (j == UINT_MAX)
			return startY - stepY;
		return startY + (j)*stepY; 
	};
	
	//! calculates (i,j) of the cell for point with coords (x,y)
	void getCoordPoint(REAL x, REAL y, size_t & i, size_t & j) const;

	//! returns i number of (i,j) of the cell for point with coords (x,y)
	size_t get_i(REAL x) const {
		if (stepX == 0)	
			return UINT_MAX;
		return (size_t)MAX(0,floor((x-startX)/stepX + 0.5));
	};

	//! returns j number of (i,j) of the cell for point with coords (x,y)
	size_t get_j(REAL y) const {
		if (stepY == 0)	
			return UINT_MAX;
		return (size_t)MAX(0,floor((y-startY)/stepY + 0.5));
	};

	//! writes tag for saving grid to \ref datafile 
	bool writeTags(datafile *df) const;

	//! returns grid name 
	const char * getName() const;

	//! sets geometry name to newname 
	void setName(const char * newname);

	//! returns number of lines need to draw geometry
	size_t getLinesCount() const;

	//! returns coordinates of line with number line_number
	void getLine(size_t line_number, float &x0, float &y0, float &x1, float &y1) const;

	//! compares grid with another 
	bool operator==(const d_grid * test) const;

	//! returns cell bounds in OX direction
	void x_from_to(size_t i, REAL & from, REAL & to) const;
	//! returns cell bounds in OY direction
	void y_from_to(size_t j, REAL & from, REAL & to) const;
	//! returns lower cell bound in OX direction
	REAL x_from(size_t i) const;
	//! returns upper cell bound in OX direction
	REAL x_to(size_t i) const;
	//! returns lower cell bound in OY direction
	REAL y_from(size_t j) const;
	//! returns upper cell bound in OY direction
	REAL y_to(size_t j) const;

	//! X-coordinate of the first node
	REAL startX;
	//! X-coordinate of the last node
	REAL endX;
	//! X-distance between first and second nodes
	REAL stepX;
	//! Y-coordinate of the first node
	REAL startY;
	//! Y-coordinate of the last node
	REAL endY;
	//! Y-distance between first and second nodes
	REAL stepY;

protected:
	
	//! grid name 
	char * gridname;

};

}; // namespace surfit;

#endif

