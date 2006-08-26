#ifndef _GDBM_FILE_H
#define _GDBM_FILE_H 1

#include <dbfile/dbfile.h>
#include <gdbm.h>

class gdbmfile : public dbfile
{
public:
						 gdbmfile (void)
						 {
						 }
						 
						~gdbmfile (void)
						 {
							if (dbopen) close ();
						 }
		
	bool		 		 open (const string &dbfile);
	void				 close (void);
				 				 
protected:
	GDBM_FILE			 f;
	
	virtual bool		 recordexists (const statstring &id);
	virtual string		*getrecord (const statstring &id);
	virtual bool		 setrecord (const statstring &id, const string &data,
									bool create=true);
	virtual bool		 removerecord (const statstring &id);
	virtual bool		 startloop (void);
	virtual bool		 nextloop (void);
	
};

#endif
