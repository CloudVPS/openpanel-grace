#ifndef _GDBM_FILE_H
#define _GDBM_FILE_H 1

#include <dbfile/dbfile.h>
#include <gdbm.h>

/// An implementation of dbfile using a GDBM backend.
class gdbmfile : public dbfile
{
public:
						 /// Default constructor.
						 gdbmfile (void)
						 {
						 }
						 
						 /// Destructor. Closes the database if it is
						 /// still open.
						~gdbmfile (void)
						 {
							if (dbopen) close ();
						 }
	
						 /// Open a GDBM database.
						 /// \param dbfile Path to the database file.
	bool		 		 open (const string &dbfile);
	
						 /// Close an already opened database.
	void				 close (void);
				 				 
protected:
	GDBM_FILE			 f; ///< Handle to the gdbm file.
	
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
