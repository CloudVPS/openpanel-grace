#include <querido/row.h>
#include <querido/table.h>

dbcell::dbcell (void)
{
	throw (dbcellDefaultConstructorException());
}

dbcell::dbcell (dbrow *prow, const statstring &pid, const value &v)
	: row (prow), id (pid)
{
	val = v;
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
	throw (dbrowDefaultConstructorException());
}

dbrow::dbrow (dbtable *tab, const statstring &pid)
	: rowid (pid)
{
	table = tab;
	fchanged = false;
	
	value tmp;
	string qry;
	qry.printf ("SELECT * FROM %s WHERE %s=\"%s\"",
				table->id().str(), table->indexcolumn().str(), pid.str());
	if (! table->eng->query (qry, tmp))
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
		if (cell.changed) res[cell.id] = cell;
	}
	
	return &res;
}

dbcell &dbrow::operator[] (const statstring &fieldname)
{
	return cells[fieldname];
}

dbrow &dbrow::operator= (const value &orig)
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

