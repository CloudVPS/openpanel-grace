// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <udbc/dbconn.h>
#include <udbc/sqlsession.h>

sqlsession::sqlsession (const string &remotehost,
						const string &table,
						const string &tsfield,
						const string &hostfield,
						const string &datafield,
						const string &idfield)
{
	_remotehost = remotehost;
	_table = table;
	_tsfield = tsfield;
	_datafield = datafield;
	_hostfield = hostfield;
	_idfield = idfield;
	_sid = 0;
	_valid = false;
}

int sqlsession::makesession (dbconnection &db, const value &data)
{
	int sid;
	string myxml;
	value rec;
	myxml = data.toxml();
	
	doexpires(db);
	
	::srand (kernel.time.now() ^ kernel.proc.self());
	
	sid = (int) ((::rand() & 0x7fffffff) ^ (_remotehost.key() & 0x7fffff));
	
	rec[_idfield] = sid;
	rec[_hostfield] = _remotehost.str();
	rec[_tsfield] = (int) (kernel.time.now() >> 2);
	rec[_datafield] = myxml;
	
	db.insert (_table, rec);
	
	_sid = sid;
	_valid = true;
	
	return sid;
}

value *sqlsession::getsession (dbconnection &db, int sid)
{
	value qry;
	value *rec = new value;
	
	doexpires(db);
	
	qry = db.queryf ("select %s from %s where id=%d and host='%S'",
					 _datafield.str(),
					 _table.str(),
					 sid,
					 _remotehost.str());
	
	if (qry.count())
	{
		string ssid;
		(*rec).fromxml (qry[0][_datafield].sval());
		_valid = true;
	}
	else
	{
		_valid = false;
	}
	
	return rec;
}

void sqlsession::updatesession (dbconnection &db, const value &v)
{
	if (! _valid) return;
	
	value rec;
	string myxml;
	
	myxml = v.toxml();
	
	rec[_idfield] = _sid;
	rec[_tsfield] = (int) (kernel.time.now() >> 2);
	rec[_datafield] = myxml;
	
	db.update (_table, rec, _idfield.str());
}

void sqlsession::killsession (dbconnection &db)
{
	if (! _valid) return;
	
	delete db.queryf ("delete from %s where %s=%d",
					  _table.str(),
					  _idfield.str(),
					  _sid);
}

bool sqlsession::validated (void)
{
	return _valid;
}

void sqlsession::doexpires (dbconnection &db)
{
	delete db.queryf ("delete from %s where %s < %d",
					  _table.str(),
					  _tsfield.str(),
					  (kernel.time.now() - 300) >> 2);
}
