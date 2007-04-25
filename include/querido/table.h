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

class dbtable
{
friend class dbcolumn;
friend class dbquery;
public:
						 dbtable (dbengine &peng, const string &tname);
						~dbtable (void);
					
	dbcolumn			&operator[] (const statstring &cname);

	dbrow				&row (const statstring &rowid);
	bool				 rowexists (const statstring &rowid);
	void				 commitrows (void);
	void				 rollback (void);

	const value			&columns (void) { return dbcolumns; }
	void				 setindexcolumn (const statstring &colid);
	
	const string		&id (void) { return name; }
	const statstring	&indexcolumn (void) { return idxid; }

	dbengine			&eng;
	
protected:
	rowdict				 rows;
	columndict			 cols;
	statstring			 idxid;
	string				 name;
	value				 dbcolumns;
};

#endif
