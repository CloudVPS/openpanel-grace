#include <querido/query.h>
#include <querido/table.h>

dbstatement::dbstatement (dbstatement *orig)
{
	l = orig->l;
	r = orig->r;
	comp = orig->comp;
	if (orig->ll) ll = new dbstatement (orig->ll);
	else ll = NULL;
	if (orig->lr) lr = new dbstatement (orig->lr);
	else lr = NULL;
}

dbstatement::dbstatement (const value &left, valuetype lefttype,
						  comptype comparison,
						  const value &right, valuetype righttype)
{
	l = left;
	r = right;
	tl = lefttype;
	tr = righttype;
	comp = comparison;
	ll = lr = NULL;
}

dbstatement::dbstatement (dbstatement *left, comptype c, dbstatement *right)
{
	tl = v_cref;
	tr = v_cref;
	
	comp = c;
	ll = new dbstatement (left);
	lr = new dbstatement (right);
	l.clear ();
	r.clear ();
}

dbstatement::~dbstatement (void)
{
	if (ll) delete ll;
	if (lr) delete lr;
}

dbstatement dbstatement::operator&& (dbstatement &right)
{
	return dbstatement (this, c_and, &right);
}

dbstatement dbstatement::operator|| (dbstatement &right)
{
	return dbstatement (this, c_or, &right);
}

string *dbstatement::sql (void)
{
	returnclass (string) res retain;
	
	res.strcat ('(');
	
	switch (tl)
	{
		case v_cref:
			res.printf ("`%s`", l.str()); break;
		
		case v_stmt:
			res.strcat (ll->sql()); break;
			
		case v_int:
			res.printf ("%i", l.ival()); break;
		
		case v_double:
			res.printf ("%f", l.dval()); break;
		
		case v_string:
			res.printf ("\"%S\"", l.cval()); break;
		
		default:
			res.printf ("NULL"); break;
	}
	
	switch (comp)
	{
		case c_eq: res.strcat ("="); break;
		case c_neq: res.strcat ("!="); break;
		case c_lt: res.strcat ("<"); break;
		case c_lte: res.strcat ("<="); break;
		case c_gte: res.strcat (">="); break;
		case c_gt: res.strcat (">"); break;
		case c_and: res.strcat (" and "); break;
		case c_or: res.strcat (" or "); break;
	}
	
	switch (tr)
	{
		case v_cref:
			res.printf ("`%s`", r.str()); break;
		
		case v_stmt:
			res.strcat (lr->sql()); break;
			
		case v_int:
			res.printf ("%i", r.ival()); break;
		
		case v_double:
			res.printf ("%f", r.dval()); break;
		
		case v_string:
			res.printf ("\"%S\"", r.cval()); break;
		
		default:
			res.printf ("NULL"); break;
	}
	
	return &res;
}

dbcolumn::dbcolumn (dbtable *ptable, const statstring &pid)
	: id (pid), table (*ptable)
{
}

dbcolumn::~dbcolumn (void)
{
}

dbstatement dbcolumn::operator== (const value &right)
{
	string tnam = name();
	
	caseselector (right.type())
	{
		incaseof (t_int):
			return dbstatement (tnam, v_cref, c_eq, right, v_int);
		
		incaseof (t_double) :
			return dbstatement (tnam, v_cref, c_eq, right, v_double);
		
		defaultcase :
			return dbstatement (tnam, v_cref, c_eq, right, v_string);
	}
}

dbstatement dbcolumn::operator!= (const value &right)
{
	string tnam = name();
	
	caseselector (right.type())
	{
		incaseof (t_int):
			return dbstatement (tnam, v_cref, c_neq, right, v_int);
		
		incaseof (t_double) :
			return dbstatement (tnam, v_cref, c_neq, right, v_double);
		
		defaultcase :
			return dbstatement (tnam, v_cref, c_neq, right, v_string);
	}
}
#include <querido/query.h>

dbstatement dbcolumn::operator< (const value &right)
{
	string tnam = name();
	
	caseselector (right.type())
	{
		incaseof (t_int):
			return dbstatement (tnam, v_cref, c_lt, right, v_int);
		
		incaseof (t_double) :
			return dbstatement (tnam, v_cref, c_lt, right, v_double);
		
		defaultcase :
			return dbstatement (tnam, v_cref, c_lt, right, v_string);
	}
}

dbstatement dbcolumn::operator<= (const value &right)
{
	string tnam = name();
	
	caseselector (right.type())
	{
		incaseof (t_int):
			return dbstatement (tnam, v_cref, c_lte, right, v_int);
		
		incaseof (t_double) :
			return dbstatement (tnam, v_cref, c_lte, right, v_double);
		
		defaultcase :
			return dbstatement (tnam, v_cref, c_lte, right, v_string);
	}
}

dbstatement dbcolumn::operator>= (const value &right)
{
	string tnam = name();
	
	caseselector (right.type())
	{
		incaseof (t_int):
			return dbstatement (tnam, v_cref, c_gte, right, v_int);
		
		incaseof (t_double) :
			return dbstatement (tnam, v_cref, c_gte, right, v_double);
		
		defaultcase :
			return dbstatement (tnam, v_cref, c_gte, right, v_string);
	}
}

dbstatement dbcolumn::operator> (const value &right)
{
	string tnam = name();
	
	caseselector (right.type())
	{
		incaseof (t_int):
			return dbstatement (tnam, v_cref, c_gt, right, v_int);
		
		incaseof (t_double) :
			return dbstatement (tnam, v_cref, c_gt, right, v_double);
		
		defaultcase :
			return dbstatement (tnam, v_cref, c_gt, right, v_string);
	}
}

dbstatement dbcolumn::operator== (dbcolumn &right)
{
	string lnam = name();
	string rnam = right.name();
	return dbstatement (lnam, v_cref, c_eq, rnam, v_cref);
}

dbstatement dbcolumn::operator!= (dbcolumn &right)
{
	string lnam = name();
	string rnam = right.name();
	return dbstatement (lnam, v_cref, c_neq, rnam, v_cref);
}

dbstatement dbcolumn::operator< (dbcolumn &right)
{
	string lnam = name();
	string rnam = right.name();
	return dbstatement (lnam, v_cref, c_lt, rnam, v_cref);
}

dbstatement dbcolumn::operator<= (dbcolumn &right)
{
	string lnam = name();
	string rnam = right.name();
	return dbstatement (lnam, v_cref, c_lte, rnam, v_cref);
}

dbstatement dbcolumn::operator>= (dbcolumn &right)
{
	string lnam = name();
	string rnam = right.name();
	return dbstatement (lnam, v_cref, c_gte, rnam, v_cref);
}

dbstatement dbcolumn::operator> (dbcolumn &right)
{
	string lnam = name();
	string rnam = right.name();
	return dbstatement (lnam, v_cref, c_gt, rnam, v_cref);
}

dbstatement dbcolumn::like (const string &right)
{
	string tnam = name();
	return dbstatement (tnam, v_cref, c_like, right, v_string);
}

string *dbcolumn::name (void)
{
	returnclass (string) res retain;
	
	res.printf ("%s.%s", table.name.str(), id.str());
	return &res;
}

dbquery::dbquery (dbengine &peng)
	: eng (peng)
{
}

dbquery::~dbquery (void)
{
}

void dbquery::where (dbstatement &st)
{
	sqlwhere = st.sql ();
}

void dbquery::select (dbtable &tab)
{
	string nwfield;
	nwfield.printf ("%s.*", tab.name.str());
	fields[nwfield] = true;
	tables[tab.name] = true;
}

void dbquery::select (dbcolumn &one)
{
	string tmp;
	
	tmp = one.name(); fields[tmp] = true; tables[one.table.name] = true;
}

void dbquery::select (dbcolumn &one, dbcolumn &two)
{
	string tmp;
	
	tmp = one.name(); fields[tmp] = true; tables[one.table.name] = true;
	tmp = two.name(); fields[tmp] = true; tables[two.table.name] = true;
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three)
{
	string tmp;
	
	tmp = one.name(); fields[tmp] = true; tables[one.table.name] = true;
	tmp = two.name(); fields[tmp] = true; tables[two.table.name] = true;
	tmp = three.name(); fields[tmp] = true; tables[three.table.name] = true;
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
					  dbcolumn &four)
{
	string tmp;
	
	tmp = one.name(); fields[tmp] = true; tables[one.table.name] = true;
	tmp = two.name(); fields[tmp] = true; tables[two.table.name] = true;
	tmp = three.name(); fields[tmp] = true; tables[three.table.name] = true;
	tmp = four.name(); fields[tmp] = true; tables[four.table.name] = true;
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
					  dbcolumn &four, dbcolumn &five)
{
	string tmp;
	
	tmp = one.name(); fields[tmp] = true; tables[one.table.name] = true;
	tmp = two.name(); fields[tmp] = true; tables[two.table.name] = true;
	tmp = three.name(); fields[tmp] = true; tables[three.table.name] = true;
	tmp = four.name(); fields[tmp] = true; tables[four.table.name] = true;
	tmp = five.name(); fields[tmp] = true; tables[five.table.name] = true;
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
					  dbcolumn &four, dbcolumn &five, dbcolumn &six)
{
	string tmp;
	
	tmp = one.name(); fields[tmp] = true; tables[one.table.name] = true;
	tmp = two.name(); fields[tmp] = true; tables[two.table.name] = true;
	tmp = three.name(); fields[tmp] = true; tables[three.table.name] = true;
	tmp = four.name(); fields[tmp] = true; tables[four.table.name] = true;
	tmp = five.name(); fields[tmp] = true; tables[five.table.name] = true;
	tmp = six.name(); fields[tmp] = true; tables[six.table.name] = true;
}

void dbquery::mksql (void)
{
	sql.crop ();
	sql.printf ("SELECT ");
	
	bool first = true;
	
	foreach (field, fields)
	{
		sql.printf ("%s`%s`", first ? "" : ",", field.name());
		first = false;
	}
	
	first = true;
	sql.printf (" FROM ");
	
	foreach (tab, tables)
	{
		sql.printf ("%s`%s`", first ? "" : ",", tab.name());
		first = false;
	}
	
	if (sqlwhere.strlen())
	{
		sql.printf (" WHERE %s", sqlwhere.str());
	}
}

value *dbquery::exec (void)
{
	returnclass (value) res retain;
	
	mksql ();
	eng.query (sql, res);
	return &res;
}