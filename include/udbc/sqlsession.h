// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _SQL_SESSION_H
#define _SQL_SESSION_H 1

#include <grace/value.h>
#include <grace/application.h>
#include <grace/system.h>
#include <udbc/dbconn.h>

class sqlsession
{
public:
					 sqlsession (const string &remotehost,
					 			 const string &table,
					 			 const string &tsfield = "ts",
					 			 const string &hostfield = "host",
					 			 const string &datafield = "data",
					 			 const string &idfield = "id");
					 			 
		int			 makesession (dbconnection &, const value &);
		value		*getsession (dbconnection &, int);
		void		 updatesession (dbconnection &, const value &);
		void		 killsession (dbconnection &);
		bool		 validated (void);
		
protected:
		void		 doexpires (dbconnection &);

		statstring	 _remotehost;
		statstring	 _hostfield;
		string		 _table;
		statstring	 _tsfield;
		statstring	 _datafield;
		statstring	 _idfield;
		int			 _sid;
		bool		 _valid;
};

#endif
