dbhandle::dbhandle (dbengine *owner)
	: drivername (owner->engine)
{
}

dbhandle::~dbhandle (void)
{
}

bool dbhandle::open (const value &details)
{
	return false;
}

bool dbhandle::close (void)
{
	return true;
}

bool dbhandle::query (const string &sql, value &into)
{
	return false;
}

bool dbhandle::listcolumns (const string &table, value &into)
{
	return false;
}

bool dbhandle:tableexists (const string &table)
{
	return false;
}

bool dbhandle::listtables (value &into)
{
	return false;
}

dbengine::dbengine (const string &driver)
{
	caseselector (driver)
	{
		incaseof ("sqlite") :
			attachbuiltin (SQLite);
			break;
		
		defaultcase :
			hdl = NULL;
			break;
	}
}

dbengine::dbengine (localengine t)
{
	attachbuiltin (t);
}

dbengine::~dbengine (void)
{
}

bool dbengine::open (const value &parm)
{
	if (! hdl) return false;
	return hdl->open (parm);
}

bool dbengine::close (void)
{
	if (! hdl) return true;
	hdl->close ();
}

bool dbengine::query (const string &sql)
{
	if (! hdl) return false;
	
	value tmp;
	return hdl->query (sql, tmp);
}

bool dbengine::query (const string &sql, value &into)
{
	if (! hdl) return false;
	
	return hdl->query (sql, into);
}

bool dbengine::listcolumns (const string &table, value &into)
{
	if (! hdl) return false;
	
	return hdl->listcolumns (table, into);
}

bool dbengine::tableexists (const string &table)
{
	if (! hdl) return false;

	return hdl->tableexists (table);
}

bool dbengine::listtables (value &into)
{
	if (! hdl) return false;
	
	return hdl->listtables (into);
}

dbcell::dbcell (dbrow *prow, const statstring &pid)
	: row (prow), id (pid)
{
}

dbcell::~dbcell (void)
{
}

dbcell &dbcell::operator= (const value &v)
{
	val = v;
	changed = true;
	row->fchanged = true;
	
	return *this;
}

bool dbcell::operator== (const value &v)
{
	return (val == v);
}

bool dbcell::operator!= (const value &v)
{
	return (val != v);
}

void dbcell::init (bool first)
{
}

void dbcell::tovalue (value &into)
{
	into = val;
}

dbrow::dbrow (void)
{
}

dbrow::dbrow (dbtable *tab, const statstring &id)
{
	table = tab;
	fchanged = false;
	
	value tmp;
	string qry;
	qry.printf ("SELECT * FROM `%s` WHERE `%s`=\"%s\"",
				table->name.str(), table->idxid.str(), id.str());
	if (! table->eng.query (qry, tmp))
	{
	}
	
	foreach (col, tmp[0])
	{
		dbcell *node = new dbcell (this, col.id(), col);
		cells.set (col.id(), node);
	}
}

dbrow::~dbrow (void)
{
	if (cells) delete cells;
}

bool dbrow::changed (void)
{
	return fchanged;
}

value *dbrow::changes (void)
{
	returnclass (value) res retain;
	
	foreach (cell, cells)
	{
		res[cell.id] = cell;
	}
	
	return &res;
}

dbcell &dbrow::operator[] (const statstring &fieldname)
{
	return cells[fieldname];
}

dbrecord &operator= (const value &orig)
{
	foreach (node, orig)
	{
		cells[node.id()] = node;
	}
	
	return *this;
}

bool dbrow::exists (const statstring &fieldname)
{
	return cells.exists (fieldname);
}

bool dbrow::changed (void)
{
	return fchanged;
}

void dbrow::init (bool first)
{
	//cells.clear ();
}

void dbrow::tovalue (value &into)
{
	foreach (cell, cells)
	{
		into[cell.id] = cell;
	}
}

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
	returnclass (res) retain;
	
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
			res.printf ("%f", l.fval()); break;
		
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
			res.printf ("%f", r.fval()); break;
		
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
	
	switch (right.itype())
	{
		case i_int:
			return dbstatement (tnam, v_cref, c_eq, right, v_int);
		
		case i_float:
			return dbstatement (tnam, v_cref, c_eq, right, v_double);
		
		default:
			return dbstatement (tnam, v_cref, c_eq, right, v_string);
	}
}

dbstatement dbcolumn::operator!= (const value &right)
{
	string tnam = name();
	
	switch (right.itype())
	{
		case i_int:
			return dbstatement (tnam, v_cref, c_neq, right, v_int);
		
		case i_float:
			return dbstatement (tnam, v_cref, c_neq, right, v_double);
		
		default:
			return dbstatement (tnam, v_cref, c_neq, right, v_string);
	}
}

dbstatement dbcolumn::operator< (const value &right)
{
	string tnam = name();
	
	switch (right.itype())
	{
		case i_int:
			return dbstatement (tnam, v_cref, c_lt, right, v_int);
		
		case i_float:
			return dbstatement (tnam, v_cref, c_lt, right, v_double);
		
		default:
			return dbstatement (tnam, v_cref, c_lt, right, v_string);
	}
}

dbstatement dbcolumn::operator<= (const value &right)
{
	string tnam = name();
	
	switch (right.itype())
	{
		case i_int:
			return dbstatement (tnam, v_cref, c_lte, right, v_int);
		
		case i_float:
			return dbstatement (tnam, v_cref, c_lte, right, v_double);
		
		default:
			return dbstatement (tnam, v_cref, c_lte, right, v_string);
	}
}

dbstatement dbcolumn::operator>= (const value &right)
{
	string tnam = name();
	
	switch (right.itype())
	{
		case i_int:
			return dbstatement (tnam, v_cref, c_gte, right, v_int);
		
		case i_float:
			return dbstatement (tnam, v_cref, c_gte, right, v_double);
		
		default:
			return dbstatement (tnam, v_cref, c_gte, right, v_string);
	}
}

dbstatement dbcolumn::operator> (const value &right)
{
	string tnam = name();
	
	switch (right.itype())
	{
		case i_int:
			return dbstatement (tnam, v_cref, c_gt, right, v_int);
		
		case i_float:
			return dbstatement (tnam, v_cref, c_gt, right, v_double);
		
		default:
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

dbtable::dbtable (dbengine &peng, const string &tname)
	: name (tname), eng (peng), idxid ("id")
{
	
}

dbtable::~dbtable (void)
{
}

dbcolumn &dbtable::operator[] (const statstring &cname)
{
	if (cols.exists (cname)) return cols[cname];
	if (! dbcolumns.exists (cname)) throw (unknownColumnException());
	
	dbcolumn *ncol = new dbcolumn (this, cname);
	cols.set (cname, ncol);
	return *ncol;
}

dbrow &dbtable::row (const statstring &rowid)
{
	if (rows.exists (rowid)) return rows[rowid];
	if (! rowexists (rowid)) throw (unknownRowException());
	
	dbrow *nrow = new dbrow (this, rowid);
	rows.set (rowid, nrow);
	return *nrow;
}

bool dbtable::rowexists (const statstring &rowid)
{
	value res;
	string qry;
	qry.printf ("SELECT `%s` FROM `%s` WHERE `%s`=\"%S\"", idxid.str(),
				name.str(), idxid.str(), rowid.str());
				
	if (! res.count()) return false;
	return true;
}

void dbtable::commitrows (void)
{
	foreach (row, rows)
	{
		if (row.changed())
		{
			value ch = row.changes ();
			string qry;
			qry.printf ("UPDATE `%s` SET ", name.str());
			bool first = true;
			foreach (change, ch)
			{
				if (! first) qry.strcat (',');
				qry.printf ("`%s`=", change.id().str());
				switch (change.itype())
				{
					case i_int:
						qry.printf ("%i", change.ival()); break;
					
					case i_double:
						qry.printf ("%f", change.dval()); break;
					
					default:
						qry.printf ("\"%S\"", change.str()); break;
				}
			}
			qry.printf (" WHERE `%s`=\"%S\"", idxid.str(), row.id().str());
			eng.query (qry);
		}
	}
	
	rollback ();
}

void dbtable::rollback (void)
{
	rows.clear ();
}

void dbtable::setindexcolumn (const statstring &colid)
{
	idxid = colid;
}
