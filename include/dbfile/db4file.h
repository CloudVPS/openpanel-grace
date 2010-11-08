// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _DB4_FILE_H
#define _DB4_FILE_H 1

#include <dbfile/dbfile.h>
#include <db.h>

/// An implementation of dbfile using the Berkeley DB4 backend.
class db4file : public dbfile
{
public:
						 /// Default constructor.
						 db4file (void)
						 {
						 }
						 
						 /// Destructor. Closes the database if it is
						 /// still open.
						~db4file (void)
						 {
							if (dbopen) close ();
						 }
	
						 /// Open a DB4 database.
						 /// \param dbfile Path to the database file.
	bool		 		 open (const string &dbfile);
	
						 /// Close an already opened database.
	void				 close (void);
				 				 
protected:
	DB					*f; ///< Handle to the db4 file.
	
						 //@{
						 /// Implementation of dbfile.
	virtual bool		 recordexists (const statstring &id);
	virtual string		*getrecord (const statstring &id);
	virtual bool		 setrecord (const statstring &id, const string &data,
									bool create=true);
	virtual bool		 removerecord (const statstring &id);
	virtual bool		 startloop (void);
	virtual bool		 nextloop (void);
	virtual bool		 filesync (void);
					 	 //@}
	
};

#endif
