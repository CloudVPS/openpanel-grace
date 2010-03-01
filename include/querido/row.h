#ifndef _QUERIDO_ROW_H
#define _QUERIDO_ROW_H 1

#include <grace/statstring.h>
#include <grace/valuable.h>
#include <grace/value.h>
#include <grace/dictionary.h>

THROWS_EXCEPTION (
	dbcellDefaultConstructorException,
	0x56a2b311,
	"Default constructor called on dbcell"
);

class dbcell : public valuable
{
friend class dbrow;
public:
					 dbcell (void);
					 dbcell (class dbrow *prow, const statstring &pid,
					 		 const value &v);
					~dbcell (void);

	dbcell			&operator= (const value &v);
	bool			 operator== (const value &v);
	bool			 operator!= (const value &v);
					 operator const string &(void) { return val.sval(); }
					 operator int (void) { return val.ival(); }
	

protected:					
	void			 init (bool first);
	void			 tovalue (value &into) const;
	
	class dbrow		*row;
	statstring		 id;
	bool			 changed;
	value			 val;
};

typedef dictionary<dbcell> celldict;

THROWS_EXCEPTION (
	dbrowDefaultConstructorException,
	0x02e22f97,
	"Default constructor called on dbrow"
);

class dbrow : public valuable
{
friend class dbcell;
public:
						 dbrow (void);
						 dbrow (class dbtable *tab, const statstring &pid);
						~dbrow (void);
					
	bool				 changed (void);
	value				*changes (void);
	
	dbcell				&operator[] (const statstring &fieldname);
	dbrow				&operator= (const value &orig);
	
	bool				 exists (const statstring &fieldname);
	const statstring	&id (void) { return rowid; }

protected:
	void				 init (bool first);
	void				 tovalue (value &into) const;
	void				 toval (value &into);
	
	statstring			 rowid;
	class dbtable		*table;
	celldict			 cells;
	bool				 fchanged;
};

typedef dictionary<dbrow> rowdict;

#endif
