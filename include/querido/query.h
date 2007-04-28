#ifndef _QUERIDO_QUERY_H
#define _QUERIDO_QUERY_H 1

#include <grace/value.h>
#include <grace/dictionary.h>
#include <querido/engine.h>

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
					 
class dbstatement
{
public:
					 dbstatement (dbstatement *orig);
					 
					 dbstatement (const value &left, valuetype lefttype,
					 			  comptype comparison,
					 			  const value &right, valuetype righttype);

					 dbstatement (dbstatement *, comptype, dbstatement *);
					
					~dbstatement (void);

	dbstatement		 operator&& (dbstatement right);
	dbstatement		 operator|| (dbstatement left);

	string			*sql (void);

	value			 l, r;
	valuetype		 tl, tr;
	comptype		 comp;
	
	dbstatement		*ll, *lr;
};

THROWS_EXCEPTION (
	dbcolumnDefaultConstructorException,
	0xf8d38258,
	"Default constructor called on dbcolumn"
);

class dbcolumn
{
friend class dbquery;
public:
					 dbcolumn (void);
					 dbcolumn (class dbtable *ptable, const statstring &pid);
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
	const string	&asname (void) { return asid; }
	
	dbcolumn		&as (const string &);
	dbcolumn		&op (const string &);
	
protected:
	statstring		 id;
	class dbtable	*table;
	string			 asid;
	string			 operation;
};

typedef dictionary<dbcolumn> columndict;

THROWS_EXCEPTION (
	mixedQueryException,
	0x48a1adcf,
	"Mixed query types in one query"
);

THROWS_EXCEPTION (
	illegalQueryOpException,
	0x30b04057,
	"Illegal operation for the query type"
);

class dbquery
{
public:
					 enum querytype
					 {
					 	q_unset,
					 	q_select,
					 	q_delete,
					 	q_update,
					 	q_insert
					 };

					 dbquery (dbengine &peng);
					~dbquery (void);
	
	void			 where (dbstatement st);
	void			 from (class dbtable &tab);
	void			 from (class dbtable &tab, class dbtable &tab);
	void			 from (class dbtable &tab, class dbtable &tab,
						   class dbtable &tab);
	void			 from (class dbtable &tab, class dbtable &tab,
						   class dbtable &tab, class dbtable &tab);
	void			 select (class dbtable &tab);
	void			 select (dbcolumn &one);
	void			 select (dbcolumn &one, dbcolumn &two);
	void			 select (dbcolumn &one, dbcolumn &two, dbcolumn &three);
	void			 select (dbcolumn &one, dbcolumn &two,
							 dbcolumn &three, dbcolumn &four);
	void			 select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
							 dbcolumn &four, dbcolumn &five);
	void			 select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
							 dbcolumn &four, dbcolumn &five, dbcolumn &six);
	dbquery			&update (class dbtable &tab);
	dbquery			&set (const value &param);
	dbquery			&deletefrom (class dbtable &tab);
	dbquery			&insertinto (class dbtable &tab);
	dbquery			&values (const value &v);
	
	dbquery			&orderby (dbcolumn &c);
	dbquery			&orderby (const string &nam);
	dbquery			&descending (void);	
	void			 indexby (const string &idxnam);
	void			 indexby (dbcolumn &c);
	
	value			*exec (void);
	//void			 execvoid (void);
	
protected:
	void			 mksql (void);
	void			 mksqlselect (void);
	void			 mksqlupdate (void);
	void			 mksqldelete (void);
	void			 mksqlinsert (void);

	string			 sql;
	string			 sqlwhere;
	value			 fields;
	value			 tables;
	value			 orderbys;
	bool			 descend;
	dbengine		&eng;
	statstring		 idxid;
	querytype		 qtype;
	value			 vset;
};

#endif
