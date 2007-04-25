#ifndef _QUERIDO_CONNECTION_H
#define _QUERIDO_CONNECTION_H 1

class dbhandle
{
public:
					 dbhandle (class dbengine *owner);
					~dbhandle (void);

	virtual bool	 open (const value &details);
	virtual bool	 close (void);
	virtual bool	 query (const string &sql, value &into);
	virtual bool	 listcolumns (const string &table, value &into);
	virtual bool	 tableexists (const string &table);
	virtual bool	 listtables (value &into);
	
	const string	&error (void) { return errstr; }
	int				 errno (void) { return errcode; }
	
protected:
	string			 drivername;
};

class dbengine
{
friend class dbhandle;
public:
					 enum localengine
					 {
					 	UDBC,
					 	MySQL,
					 	SQLite
					 };

					 dbengine (const string &driver);
					 dbengine (localengine builtintype);
					~dbengine ();
					
	bool			 open (const value &details);
	bool			 close (void);
	bool			 query (const string &sql);
	bool			 query (const string &sql, value &into);
	bool			 listcolumns (const string &table, value &into);
	bool			 tableexists (const string &table);
	bool			 listtables (value &into);

protected:
	dbhandle		*hdl;
	string			 engine;
	
	void			 attachbuiltin (localengine t);
};

class dbcell : public valuable
{
friend class dbrow;
public:
					 dbcell (class dbrow *prow, const statstring &pid,
					 		 const value &v);
					~dbcell (void);

	dbcell			&operator= (const value &v);
	bool			 operator== (const value &v);
	bool			 operator!= (const value &v);
	

protected:					
	void			 init (bool first);
	void			 tovalue (value &into);
	
	class dbrow		*row;
	statstring		 id;
	bool			 changed;
	value			 val;
};

typedef dictionary<dbcell> celldict;

class dbrow : public valuable
{
friend class dbcell;
public:
					 dbrow (void);
					 dbrow (dbtable *tab, const statstring &id);
					~dbrow (void);
					
	bool			 changed (void);
	value			*changes (void);
	
	dbcell			&operator[] (const statstring &fieldname);
	dbrecord		&operator= (const value &orig);
	
	bool			 exists (const statstring &fieldname);

protected:
	void			 init (bool first);
	void			 tovalue (value &into);
	
	dbtable			*table;
	celldict		 cells;
	bool			 fchanged;
};

typedef dictionary<dbrow> rowdict;

class dbstatement
{
public:
					 enum valuetype
					 {
					 	v_cref,
					 	v_stmt,
					 	v_int,
					 	v_double,
					 	v_string
					 };
					 
					 enum comptype
					 {
					 	c_eq,
					 	c_neq,
					 	c_lt,
					 	c_lte,
					 	c_gte,
					 	c_gt,
					 	c_like,
					 	c_and,
					 	c_or
					 };
					 
					 dbstatement (const value &left, valuetype lefttype,
					 			  comptype comparison,
					 			  const value &right, valuetype righttype);

					 dbstatement (dbstatement *, comptype, dbstatement *);
					
					~dbstatement (void);

	dbstatement		 operator&& (dbstatement &right);
	dbstatement		 operator|| (dbstatement &left);

	string			*sql (void);

	value			 l, r;
	valuetype		 tl, tr;
	comparison		 comp;
	
	dbstatement		*ll, *lr;
};

class dbcolumn
{
friend class dbquery;
public:
					 dbcolumn (dbtable *ptable, const statstring &pid);
					~dbcolumn (void);
					
	dbstatement		 operator== (const value &);
	dbstatement		 operator== (dbcolumn &);
	dbstatement		 operator!= (const value &);
	dbstatement		 operator!= (dbcolumn &);
	dbstatement		 operator<  (const value &);
	dbstatement		 operator<  (dbcolumn &);
	dbstatement		 operator<= (const value &);
	dbstatement		 operator<= (dbcolumn &);
	dbstatement		 operator>= (const value &);
	dbstatement		 operator>= (dbcolumn &);
	dbstatement		 operator>  (const value &);
	dbstatement		 operator>  (dbcolumn &);
	dbstatement		 like		(const string &);
	
	string			*name (void);
	
protected:
	statstring		 id;
	dbtable			&table;
};

class dbquery
{
public:
					 dbquery (dbengine &peng);
					~dbquery (void);
	
	void			 where (dbstatement &st);
	void			 select (dbtable &tab);
	void			 select (dbcolumn &one);
	void			 select (dbcolumn &one, dbcolumn &two);
	void			 select (dbcolumn &one, dbcolumn &two, dbcolumn &three);
	void			 select (dbcolumn &one, dbcolumn &two,
							 dbcolumn &three, dbcolumn &four);
	void			 select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
							 dbcolumn &four, dbcolumn &five);
	void			 select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
							 dbcolumn &four, dbcolumn &five, dbcolumn &six);
	
	value			*exec (void);
	//void			 execvoid (void);
	
protected:
	void			 mksql (void);

	string			 sql;
	string			 sqlwhere;
	value			 fields;
	value			 tables;
	dbengine		&eng;
};

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
					
	dbcolumn		&operator[] (const statstring &cname);

	dbrow			&row (const statstring &rowid);
	bool			 rowexists (const statstring &rowid);
	void			 commitrows (void);
	void			 rollback (void);

	const value		&columns (void) { return dbcolumns; }
	void			 setindexcolumn (const statstring &colid);
	
protected:
	rowdict			 rows;
	columndict		 cols;
	statstring		 idxid;
	string			 name;
	value			 dbcolumns;
	
	dbengine		&eng;
};

#endif
