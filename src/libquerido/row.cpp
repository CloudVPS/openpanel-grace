// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

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
	changed = false;
}

dbcell::~dbcell (void)
{
}

dbcell &dbcell::operator= (const value &v)
{
	if (v != val)
	{
		val = v;
		changed = true;
		row->fchanged = true;
	}
	
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

void dbcell::tovalue (value &into) const
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
	string qry = "SELECT * FROM %s WHERE %s=\"%s\"" %format (table->id(),
							table->indexcolumn(), pid);

	if (! table->eng->query (qry, tmp))
	{
	}
	
	string tj = tmp[0].tojson();
	
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

void dbrow::tovalue (value &into) const
{
	return ((dbrow*)this)->toval (into);
}

void dbrow::toval (value &into)
{
	foreach (cell, cells)
	{
		into[cell.id] = cell;
	}
}

