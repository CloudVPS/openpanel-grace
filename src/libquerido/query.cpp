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
	tl = orig->tl;
	tr = orig->tr;
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
	tl = v_stmt;
	tr = v_stmt;
	
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

dbstatement dbstatement::operator&& (dbstatement right)
{
	return dbstatement (this, c_and, &right);
}

dbstatement dbstatement::operator|| (dbstatement right)
{
	return dbstatement (this, c_or, &right);
}

string *dbstatement::sql (bool isselect)
{
	returnclass (string) res retain;
	
	res.strcat ('(');
	string tstr;
	
	string lstr = l.sval();
	if ((! isselect) && (lstr.strchr ('.') >= 0)) lstr = lstr.cutafter ('.');
	
	string rstr = r.sval();
	if ((! isselect) && (rstr.strchr ('.') >= 0)) rstr = rstr.cutafter ('.');
	
	switch (tl)
	{
		case v_cref:
			res.strcat (lstr); break;
		
		case v_stmt:
			res.strcat (ll->sql(isselect)); break;
			
		case v_int:
			res.strcat ("%i" %format (l)); break;
		
		case v_double:
			res.strcat ("%f" %format (l)); break;
		
		case v_string:
			res.strcat ("\"%S\"" %format (l)); break;
		
		default:
			res.strcat ("NULL"); break;
	}
	
	switch (comp)
	{
		case c_eq: res.strcat ("="); break;
		case c_neq: res.strcat ("!="); break;
		case c_lt: res.strcat ("<"); break;
		case c_lte: res.strcat ("<="); break;
		case c_gte: res.strcat (">="); break;
		case c_gt: res.strcat (">"); break;
		case c_and: res.strcat (" AND "); break;
		case c_or: res.strcat (" OR "); break;
	}
	
	switch (tr)
	{
		case v_cref:
			res.strcat (rstr); break;
		
		case v_stmt:
			res.strcat (lr->sql(isselect)); break;
			
		case v_int:
			res.strcat ("%i" %format (r)); break;
		
		case v_double:
			res.strcat ("%f" %format (r)); break;
		
		case v_string:
			res.strcat ("\"%S\"" %format (r)); break;
		
		default:
			res.strcat ("NULL"); break;
	}
	
	res.strcat (')');
	
	return &res;
}

dbcolumn::dbcolumn (void)
	: id (""), table (NULL)
{
	throw (dbcolumnDefaultConstructorException());
}

dbcolumn::dbcolumn (dbtable *ptable, const statstring &pid)
	: id (pid), table (ptable)
{
	asid = id;
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

dbcolumn &dbcolumn::as (const string &p)
{
	asid = p;
	return *this;
}

dbcolumn &dbcolumn::op (const string &p)
{
	operation = p;
	return *this;
}

string *dbcolumn::name (void)
{
	returnclass (string) res retain;
	
	if (operation) res.strcat ("%s(" %format (operation));
	res.strcat ("%s.%s" %format (table->name, id));
	if (operation) res.strcat (')');
	return &res;
}

dbquery::dbquery (dbengine &peng)
	: eng (peng)
{
	descend = false;
	idxid = "id";
	qtype = q_unset;
	limitamount = limitoffs = 0;
}

dbquery::~dbquery (void)
{
}

void dbquery::where (dbstatement st)
{
	if ((qtype == q_unset)||(qtype == q_insert))
		throw (illegalQueryOpException());
		
	sqlwhere = st.sql (qtype == q_select);
}

void dbquery::from (dbtable &tab)
{
	if ((qtype == q_update) || (qtype == q_insert))
	{
		throw (illegalQueryOpException());
	}
	tables[tab.name] = true;
}

void dbquery::from (dbtable &tab1, dbtable &tab2)
{
	from (tab1);
	from (tab2);
}

void dbquery::from (dbtable &tab1, dbtable &tab2, dbtable &tab3)
{
	from (tab1);
	from (tab2);
	from (tab3);
}

void dbquery::from (dbtable &tab1, dbtable &tab2, dbtable &tab3, dbtable &tab4)
{
	from (tab1);
	from (tab2);
	from (tab3);
	from (tab4);
}

void dbquery::select (dbtable &tab)
{
	if ( (qtype != q_unset) && (qtype != q_select) )
	{
		throw (mixedQueryException());
	}
	qtype = q_select;
	string nwfield = "%s.*" %format (tab.name);
	fields[nwfield] = nwfield;
	tables[tab.name] = true;
}

void dbquery::select (dbcolumn &one)
{
	if ( (qtype != q_unset) && (qtype != q_select) )
	{
		throw (mixedQueryException());
	}
	qtype = q_select;
	
	string tmp;
	tmp = one.name(); fields[tmp] = one.asname(); tables[one.table->name] = true;
}

void dbquery::select (dbcolumn &one, dbcolumn &two)
{
	select (one);
	select (two);
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three)
{
	select (one);
	select (two);
	select (three);
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
					  dbcolumn &four)
{
	select (one);
	select (two);
	select (three);
	select (four);
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
					  dbcolumn &four, dbcolumn &five)
{
	select (one);
	select (two);
	select (three);
	select (four);
	select (five);
}

void dbquery::select (dbcolumn &one, dbcolumn &two, dbcolumn &three,
					  dbcolumn &four, dbcolumn &five, dbcolumn &six)
{
	select (one);
	select (two);
	select (three);
	select (four);
	select (five);
	select (six);
}

dbquery &dbquery::orderby (dbcolumn &one)
{
	if (qtype != q_select) throw (illegalQueryOpException());
	
	orderbys.newval() = one.name();
	return *this;
}

dbquery &dbquery::orderby (const string &one)
{
	if (qtype != q_select) throw (illegalQueryOpException());

	orderbys.newval() = one;
	return *this;
}

dbquery &dbquery::descending (void)
{
	if (qtype != q_select) throw (illegalQueryOpException());
	descend = true;
	return *this;
}

dbquery &dbquery::limit (int offset, int amount)
{
	limitoffs = offset	;
	limitamount = amount;
	return *this;
}

void dbquery::indexby (const string &idxnam)
{
	if (qtype != q_select) throw (illegalQueryOpException());
	idxid = idxnam;
}

void dbquery::indexby (dbcolumn &one)
{
	if (qtype != q_select) throw (illegalQueryOpException());
	idxid = one.asname();
}

dbquery &dbquery::update (dbtable &tab)
{
	if (qtype != q_unset) throw (mixedQueryException());
	tables[tab.name] = true;
	qtype = q_update;
	return *this;
}

dbquery &dbquery::set (const value &v)
{
	if (qtype != q_update) throw (illegalQueryOpException());
	vset = v;
	return *this;
}

dbquery &dbquery::insertinto (dbtable &tab)
{
	if (qtype != q_unset) throw (illegalQueryOpException());
	tables[tab.name] = true;
	qtype = q_insert;
	return *this;
}

dbquery &dbquery::values (const value &v)
{
	if (qtype != q_insert) throw (illegalQueryOpException());
	vset = v;
	return *this;
}

dbquery &dbquery::deletefrom (dbtable &tab)
{
	if (qtype != q_unset) throw (illegalQueryOpException());
	tables[tab.name] = true;
	qtype = q_delete;
	return *this;
}

void dbquery::mksql (void)
{
	switch (qtype)
	{
		case q_select: mksqlselect(); break;
		case q_insert: mksqlinsert(); break;
		case q_delete: mksqldelete(); break;
		case q_update: mksqlupdate(); break;
	}
}

void dbquery::mksqlselect (void)
{
	sql.crop ();
	sql.printf ("SELECT ");
	
	bool first = true;
	
	foreach (field, fields)
	{
		if (field.sval().strchr ('*') < 0)
			sql.strcat ("%s%s AS %s" %format (first?"":", ", field.id(), field));
		else
			sql.strcat ("%s%s" %format (first ? "" : ", ", field.id()));
			
		first = false;
	}
	
	first = true;
	sql.strcat (" FROM ");
	
	foreach (tab, tables)
	{
		sql.strcat ("%s%s" %format (first ? "" : ",", tab));
		first = false;
	}
	
	if (sqlwhere.strlen())
	{
		sql.strcat (" WHERE %s" %format (sqlwhere));
	}
	
	if (orderbys.count())
	{
		first = true;
		sql.strcat (" ORDER BY ");
		foreach (o, orderbys)
		{
			sql.strcat ("%s%s" %format (first ? "" : ",", o));
			first = false;
		}
		if (descend) sql.strcat (" DESC");
	}
	
	if (limitamount || limitoffs)
	{
		sql.strcat (" LIMIT %i,%i" %format (limitoffs, limitamount));
	}
}

void dbquery::mksqlupdate (void)
{
	string maintable = tables[0].id();
	sql = "UPDATE %s SET " %format (maintable);

	bool first = true;
	foreach (v, vset)
	{
		if (! first) sql.strcat (',');
		first = false;
		
		sql.strcat ("%s=" %format (v.id()));
		caseselector (v.type())
		{
			incaseof (t_int):
				sql.strcat ("%i" %format (v));
				break;
			
			incaseof (t_bool):
				sql.strcat ("%i" %format (v.bval() ? 1 : 0));
				break;
			
			incaseof (t_double):
				sql.strcat ("%f" %format (v));
				break;
			
			defaultcase :
				sql.strcat ("\"%S\"" %format (v));
				break;
		}
	}
	
	if (sqlwhere.strlen())
	{
		sql.strcat (" WHERE %s" %format (sqlwhere));
	}
}

void dbquery::mksqldelete (void)
{
	sql = "DELETE FROM %s" %format (tables[0].id());
	if (sqlwhere.strlen()) sql.strcat (" WHERE %s" %format (sqlwhere));
}

void dbquery::mksqlinsert (void)
{
	sql = "INSERT INTO %s(" %format (tables[0].id());
	
	bool first = true;
	foreach (v, vset)
	{
		if (! first) sql.strcat (',');
		first = false;
		sql.strcat (v.id().sval());
	}
	
	sql.strcat (") VALUES (");
	
	first = true;
	foreach (v, vset)
	{
		if (! first) sql.strcat (',');
		first = false;
		
		caseselector (v.type())
		{
			incaseof (t_int):
				sql.strcat ("%i" %format (v));
				break;
			
			incaseof (t_bool):
				sql.strcat ("%i" %format (v.bval() ? 1 : 0));
				break;
			
			incaseof (t_double):
				sql.strcat ("%f" %format (v));
				break;
			
			defaultcase :
				sql.strcat ("\"%S\"" %format (v));
				break;
		}
	}
	
	sql.strcat (")");
}

value *dbquery::exec (void)
{
	returnclass (value) res retain;
	
	mksql ();
	eng.query (sql, res, idxid);
	return &res;
}

bool dbquery::execvoid (void)
{
	mksql ();
	return eng.query (sql, idxid);
}
