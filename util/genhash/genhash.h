#ifndef _genhash_H
#define _genhash_H 1
#include <grace/application.h>

//  -------------------------------------------------------------------------
/// Main application class.
//  -------------------------------------------------------------------------
class genhashApp : public application
{
public:
		 	 genhashApp (void) :
				application ("nl.madscience.grace.tools.genhash")
			 {
			 }
			~genhashApp (void)
			 {
			 }

	int		 main (void);
	void	 printhash (const string &type, const string &data);
};

#endif
