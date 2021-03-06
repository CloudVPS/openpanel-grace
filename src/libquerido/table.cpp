// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <querido/table.h>
#include <querido/row.h>
#include <querido/query.h>

dbtable::dbtable (void)
	: idxid ("id")
{
	eng = NULL;
}

dbtable::dbtable (dbengine &peng, const string &tname)
	: name (tname), eng (&peng), idxid ("id")
{
	if (! eng->listcolumns (tname, dbcolumns))
	{
		throw (tableInfoException());
	}
}

dbtable::~dbtable (void)
{
}

void dbtable::attach (dbengine &peng, const string &tname)
{
	name = tname;
	eng = &peng;
	if (! eng->listcolumns (tname, dbcolumns))
	{
		throw (tableInfoException());
	}
}

dbcolumn &dbtable::operator[] (const statstring &cname)
{
	if (cols.exists (cname)) return cols[cname];
	if (! dbcolumns.exists (cname))
	{
		string dbcollisting;
		dbcollisting = dbcolumns.encode ();
		throw (unknownColumnException());
	}
	
	dbcolumn *ncol = new dbcolumn (this, cname);
	cols.set (cname, ncol);
	return *ncol;
}

dbrow &dbtable::row (const statstring &rowid)
{
	if (rows.exists (rowid))
	{
		return rows[rowid];
	}

	if (! rowexists (rowid)) 
	{
		throw (unknownRowException());
	}
	
	dbrow *nrow = new dbrow (this, rowid);
	rows.set (rowid, nrow);
	return *nrow;
}

bool dbtable::rowexists (const statstring &rowid)
{
	if (rows.exists (rowid)) return true;
	value res;	
	string qry;
	qry.printf ("SELECT %s FROM %s WHERE %s=\"%S\"", idxid.str(),
				name.str(), idxid.str(), rowid.str());
	
	eng->query (qry, res);
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
			qry.printf ("UPDATE %s SET ", name.str());
			bool first = true;
			foreach (change, ch)
			{
				if (! first) qry.strcat (',');
				first = false;
				qry.printf ("%s=", change.id().str());
				caseselector (change.type())
				{
					incaseof (t_int) :
						qry.printf ("%i", change.ival()); break;
					
					incaseof (t_double) :
						qry.printf ("%f", change.dval()); break;
					
					defaultcase :
						qry.printf ("\"%S\"", change.str()); break;
				}
			}
			qry.printf (" WHERE %s=\"%S\"", idxid.str(), row.id().str());
			eng->query (qry);
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
