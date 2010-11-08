// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _QUERIDO_TABLE_H
#define _QUERIDO_TABLE_H 1

#include <querido/engine.h>
#include <querido/row.h>
#include <querido/query.h>

THROWS_EXCEPTION (
	unknownColumnException,
	0x13ba1891,
	"Column does not exist"
);

THROWS_EXCEPTION (
	unknownRowException,
	0x70f26ea2,
	"Row does not exist"
);

THROWS_EXCEPTION (
	tableInfoException,
	0x4fc5f877,
	"Error getting table information"
);

class dbtable
{
friend class dbcolumn;
friend class dbquery;
public:
						 dbtable (void);
						 dbtable (dbengine &peng, const string &tname);
						~dbtable (void);
	
	void				 attach (dbengine &peng, const string &tname);
	dbcolumn			&operator[] (const statstring &cname);

	dbrow				&row (const statstring &rowid);
	bool				 rowexists (const statstring &rowid);
	void				 commitrows (void);
	void				 rollback (void);

	const value			&columns (void) { return dbcolumns; }
	void				 setindexcolumn (const statstring &colid);
	
	const string		&id (void) { return name; }
	const statstring	&indexcolumn (void) { return idxid; }

	dbengine			*eng;
	
protected:
	rowdict				 rows;
	columndict			 cols;
	statstring			 idxid;
	string				 name;
	value				 dbcolumns;
};

#endif
