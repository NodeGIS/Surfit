
/*------------------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2002-2006 by M. V. Dmitrievsky
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

#include "surfit_io_ie.h"
#include "surf_io.h"

#include "interp.h"

// sstuff includes
#include "sstuff.h"

// surfit includes
#include "curv.h"
#include "fileio.h"
#include "vec.h"
#include "variables_tcl.h"

#include "shapelib/shapefil.h"

namespace surfit {

bool _curv_save_shp(const d_curv * contour, const char * filename) {

	
	if (!contour->getName())
		writelog(LOG_MESSAGE,"saving curv to ERSI shape file %s", filename);
	else
		writelog(LOG_MESSAGE,"saving curv \"%s\" to ERSI shape file %s", contour->getName(), filename);

	if (!contour) {
		writelog(LOG_WARNING, "NULL pointer to curv.");
		return false;
	};

	DBFHandle hDBF;
	SHPHandle hSHP;
	
	hSHP = SHPOpen( filename, "rb+" );
	hDBF = DBFOpen( filename, "rb+" );

	if (hSHP == NULL || hDBF == NULL) {

		if (hSHP)
			SHPClose( hSHP );
		if (hDBF)
			DBFClose( hDBF );


		hSHP = SHPCreate( filename, SHPT_ARC );
		
		if( hSHP == NULL ) {
			writelog(LOG_ERROR, "Unable to create:%s", filename );
			return false;
		}
		
		hDBF = DBFCreate( filename );
	}

	int shpType;
	SHPGetInfo(hSHP, NULL, &shpType, NULL, NULL);

	if (shpType != SHPT_ARC) {
		writelog(LOG_ERROR, "%s : Wrong shape type! Should be SHPT_ARC.", filename);
		SHPClose( hSHP );
		DBFClose( hDBF );
		return false;
	}

	int name_field = DBFGetFieldIndex( hDBF, "NAME" );

	if (name_field == -1) {
		if( DBFAddField( hDBF, "NAME", FTString, 50, 0 ) == -1 )
		{
			writelog(LOG_ERROR, "DBFAddField(%s,FTString) failed.", "NAME");
			SHPClose( hSHP );
			DBFClose( hDBF );
			return false;
		}
		name_field = DBFGetFieldIndex( hDBF, "NAME" );
	};

	std::vector<REAL> data_x(contour->size());
	std::vector<REAL> data_y(contour->size());
	std::copy(contour->X->begin(), contour->X->end(), data_x.begin());
	std::copy(contour->Y->begin(), contour->Y->end(), data_y.begin());


	SHPObject * psObject = SHPCreateObject(SHPT_ARC,
			-1, 0, NULL, NULL, contour->size(), 
			&*(data_x.begin()), &*(data_y.begin()), NULL, NULL);

	SHPComputeExtents(psObject);
		
	int pos = SHPWriteObject(hSHP, -1, psObject);
		
	SHPDestroyObject(psObject);

	if (contour->getName())
		DBFWriteStringAttribute(hDBF, pos, name_field, contour->getName() );

	SHPClose( hSHP );
	DBFClose( hDBF );

	return true;

};

bool _curv_load_shp(const char * filename, const char * curvname) {

	SHPHandle hSHP;
	DBFHandle hDBF;
	
	hSHP = SHPOpen(filename, "rb");
	if( hSHP == NULL ) {
		writelog(LOG_ERROR, "Unable to open:%s", filename );
		return false;
	}

	int shpType;
	int Entities;
	SHPGetInfo(hSHP, &Entities, &shpType, NULL, NULL);

	if (shpType != SHPT_ARC) {
		SHPClose( hSHP );
		writelog(LOG_ERROR, "%s : Wrong shape type! Should be SHTP_ARC.", filename);
		return false;
	}

	hDBF = DBFOpen(filename, "rb");
	if( hDBF == NULL ) {
		SHPClose(hSHP);
		writelog(LOG_ERROR, "Unable to open DBF for %s", filename );
		return false;
	}

	int name_field = DBFGetFieldIndex( hDBF, "NAME" );
	int dbf_records = DBFGetRecordCount( hDBF );

	Entities = MIN(dbf_records, Entities);

	int i, j;
	for (i = 0; i < Entities; i++) {
		SHPObject * shpObject = SHPReadObject( hSHP, i );

		if (shpObject->nParts != 1) {
			SHPDestroyObject(shpObject);
			continue;
		}
			
		const char * name = NULL;
		if (name_field != -1)
			name = DBFReadStringAttribute( hDBF, i, name_field );

		if (curvname != NULL) {
			if ( StringMatch(curvname, name) == false )
				continue;
		}

		writelog(LOG_MESSAGE,"loading curve \"%s\" from ESRI shape file %s",
			name?name:"noname",filename);

		vec * X = create_vec(shpObject->nVertices,0,0);
		vec * Y = create_vec(shpObject->nVertices,0,0);
		
		for (j = 0; j < shpObject->nVertices; j++) {
			(*X)(j) = *(shpObject->padfX + j);
			(*Y)(j) = *(shpObject->padfY + j);
		}
		
		d_curv * res = create_curv(X, Y, name);

		surfit_curvs->push_back(res);
	}

	DBFClose( hDBF );
	SHPClose( hSHP );

	return true;
};

};

