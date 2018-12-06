
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
#include "area_internal.h"
#include "area.h"
#include "curv.h"
#include "curv_internal.h"
#include "../sstuff/boolvec.h"
#include "../sstuff/datafile.h"
#include "curv_internal.h"

#include <errno.h>
#include <vector>

namespace surfit {

d_area * _area_read(const char * filename, const char * areaname,
		    int col1, int col2, int skip_lines,
		    int grow_by, const char * mask) 
{
	d_curv * crv = _curv_read(filename, areaname, col1, col2, skip_lines, grow_by, mask);
	if (crv == NULL)
		return NULL;

	d_area * res = create_area(crv, 1, areaname);
	return res;
};

bool _area_write(const d_area * area, const char * filename, const char * mask) {
	if (!area)
		return false;
	if (area->size() != 1) {
		writelog(LOG_ERROR,"area consists > 1 curves!");
		return false;
	}
	return _curv_write( (*(area->curves))[0], filename, mask);
};

bool _area_save(const d_area * area, const char * filename) {
	if (!area) {
		writelog(LOG_WARNING, "NULL pointer to area.");
		return false;
	};

	datafile *df = new datafile(filename, DF_MODE_WRITE); // write
	if (!df->condition())
		return false;

	bool res = _area_save_df(area, df);

	bool op = df->writeEof(); res = (op && res);
	
	delete df;
	return res;
};

bool _area_save_df(const d_area * area, datafile * df) {

	if (!area->getName())
		writelog(LOG_MESSAGE,"saving area to file %s", df->get_filename());
	else
		writelog(LOG_MESSAGE,"saving area \"%s\" to file %s", area->getName(), df->get_filename());
	
	bool res = true;
	bool op;

	op = df->writeTag("area");					 res = (op && res);
	if (area->getName()) {
		op = df->writeString("name", area->getName());		 res = (op && res);
	}
	int i;
	for (i = 0; i < area->size(); i++) {
		const d_curv * crv = area->get_curv(i);
		op = _curv_save_df(crv, df);				 res = (op && res);
	}
	
	op = df->writeBoolArray("orients", area->orients); res = (op && res);

	op = df->writeEndTag();						 res = (op && res);
		
	return res;
};

d_area * _area_load_df(datafile * df, const char * areaname) {

	if (!areaname) 
		writelog(LOG_MESSAGE,"loading area with no name from file %s",df->get_filename());
	else 
		writelog(LOG_MESSAGE,"loading area \"%s\" from file %s",areaname,df->get_filename());

	if (!df->condition()) {
		return NULL;
	}

	char error[] = "area_load : wrong datafile format";

	bool err = false;
	d_area * area = NULL;
	std::vector<d_curv *> * area_data = new std::vector<d_curv *>;
	//boolvec * area_orients = new boolvec;
	boolvec * orients = NULL;
	char * name = NULL;
	
	bool loaded = false;
	
	while ( !loaded ) {
		
		if (df->findTag("area")) {
			
			df->skipTagName();
			if (!df->readWord()) goto exit;
			
			while ( !df->isWord("endtag" ) ) {
				
				if ( df->isWord("char") ) {
					// read char name
					if (!df->readWord()) goto exit;
					if ( df->isWord("name") ) {
						if ( !df->readWord() ) goto exit;
						name = (char*)malloc(strlen(df->getWord())+1);
						strcpy(name,df->getWord());
						if (!df->readWord()) goto exit;
						continue;
					}
				}

				if ( df->isWord("tag") ) {
					if (!df->readWord()) goto exit;
					if ( df->isWord("curv") ) {
						d_curv * crv = NULL;
						if ( !_curv_load_df_tag_readed(df, crv) ) goto exit;
						if (crv)
							area_data->push_back(crv);
						goto cont;
					}
					if (!df->skipToEndTag()) goto exit;
					goto cont;
				}

				if ( df->isWord("array") ) {
					if (!df->readWord()) goto exit;
					if ( df->isWord("bool") ) {
						if (!df->readWord()) goto exit;
						if ( df->isWord("orients") ) {
							df->readBoolArray(orients);
							goto cont;
						}
					}
					if ( !df->skipArray(false) ) goto exit;
				}
								
				if ( !df->skip(false) ) goto exit;
				goto cont;
cont:
				if ( !df->readWord() ) goto exit;
				continue;
			}
		} else {
			goto exit;
		}
				
		if (err) {
			free(name);
			return NULL;
		}
		
		area = create_area(area_data, orients, name);
		free(name);
		
		if (!areaname) {
			return area;
		} else {
			if (area->getName()) {
				if (strcmp(area->getName(),areaname) == 0) {
					return area;
				}
			}
			if (area)
				area->release();
			area = NULL;
			area_data = NULL;
			area_data = new std::vector<d_curv *>;
			orients = NULL;
		}
		
	}
	
	return area;

exit:

	if (!areaname)
		writelog(LOG_ERROR, "area_load : this file have no area");
	else 
		writelog(LOG_ERROR, "area_load : this file have no area named %s", areaname);
	return NULL;

};

d_area * _area_load(const char * filename, const char * areaname) {

	datafile * df = new datafile(filename, DF_MODE_READ);
	
	d_area * arealine = _area_load_df(df, areaname);

	if (!arealine)
		goto exit;

	delete df;
	return arealine;
	
exit:
	if (arealine)
		arealine->release(); 
	delete df;
	return NULL;

};

void _area_info(const d_area * arealine) {
	if (!arealine) {
		writelog(LOG_WARNING, "_area_info : NULL pointer given");
		return;
	}
	writelog(LOG_MESSAGE,"area (%s)",arealine->getName());
};


}; // namespace surfit;

