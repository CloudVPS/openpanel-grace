// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <querido/sqlite.h>

#ifdef HAVE_SQLITE3

sqlitehandle::sqlitehandle (dbengine *owner)
	: dbhandle (owner)
{
	hdl = NULL;
}

sqlitehandle::~sqlitehandle (void)
{
	if (hdl) close ();
}

bool sqlitehandle::open (const value &details)
{
	if (sqlite3_open(details["path"].cval(), &hdl) != SQLITE_OK)
		return false;
	
	return true;
}

bool sqlitehandle::close (void)
{
	if (! hdl) return true;
	
	if (sqlite3_close (hdl) == SQLITE_OK)
	{
		hdl = NULL;
		return true;
	}
	
	return false;
}

bool sqlitehandle::query (const string &sql, value &into,
						  const statstring &indexby)
{
	sqlite3_stmt *qhandle;
	int qres;
	int rowcount=0;
	int colcount;
	int i;
	statstring curidx;
	bool done = false;
	
	into.clear ();
	
	if (sqlite3_prepare (hdl, sql.str(), -1, &qhandle, 0) != SQLITE_OK)
	{
		errcode = 1;
		errstr = "Could not prepare: %s" %format (sqlite3_errmsg(hdl));
		return false;
	}
	
	if (! (qres = sqlite3_step (qhandle)))
	{
		errcode = 1;
		errstr = "Error making first step: %s" %format (sqlite3_errmsg(hdl));
		sqlite3_finalize (qhandle);
		return false;
	}
	
	colcount = sqlite3_column_count (qhandle);
	if (colcount == 0)
	{
		into("rowschanged") = sqlite3_changes (hdl);
		sqlite3_finalize (qhandle);
		return true;
	}
	
	statstring colnames[colcount];
	value colidx;
	
	for (i=0; i<colcount; ++i)
	{
		colnames[i] = sqlite3_column_name (qhandle, i);
		colidx[colnames[i]] = i;
	}
	
	int indexfield = -1;
	if (colidx.exists (indexby)) indexfield = colidx[indexby];
	
	if (! done) do
	{
		switch (qres)
		{
			case SQLITE_BUSY:
				sleep (1);
				continue;
			
			case SQLITE_MISUSE: // achtung, fallthrough
			case SQLITE_ERROR:
				errcode = 1;
				errstr = "Error in sqlite3_step: %s" %format (sqlite3_errmsg(hdl));
				done = true;
				break;
			
			case SQLITE_DONE:
				done = true;
				break;
				
			case SQLITE_ROW:
				{
					if (indexfield>=0)
					{
						curidx = sqlite3_column_text (qhandle, indexfield);
					}
					
					value &myrow = (indexfield<0) ? into.newval() : into[curidx];
					
					for (int i=0; i<colcount; i++)
					{
						int ctype = sqlite3_column_type (qhandle, i);
						statstring &curcol = colnames[i];
						
						switch (ctype)
						{
							case SQLITE_INTEGER:
								myrow[curcol] = sqlite3_column_int (qhandle, i);
								break;
							
							case SQLITE_FLOAT:
								myrow[curcol] = sqlite3_column_double (qhandle, i);
								break;
							
							case SQLITE_BLOB:
								// FIXME: use sqlite3_column_blob
							case SQLITE_TEXT:
								myrow[curcol] = sqlite3_column_text (qhandle, i);
								break;
							
							default:
								break;
						}
					}
				}
				break;
				
		}
		rowcount++;
	} while ((qres = sqlite3_step (qhandle)) && !done);
	
	if (sqlite3_finalize (qhandle) != SQLITE_OK)
	{
		errcode = 1;
		errstr = "Error finalizing: %s" %format (sqlite3_errmsg(hdl));
		return false;
	}
	
	into ("insertid") = sqlite3_last_insert_rowid (hdl);
	into.type (t_dict);
	string tmp = into.tojson();
	return true;
}

bool sqlitehandle::listcolumns (const string &table, value &into)
{
	string qry = "PRAGMA table_info(%s)" %format (table);
	return (query (qry, into, "name"));
}

bool sqlitehandle::tableexists (const string &table)
{
	value tmp;
	listcolumns (table, tmp);
	return (tmp.count());
}

bool sqlitehandle::listtables (value &into)
{
	string qry = "SELECT name FROM sqlite_master WHERE type=\"table\"";
	return (query (qry, into, "name"));
}

int sqlitehandle::rowsaffected (void)
{
	return sqlite3_changes (hdl);
}

#endif
