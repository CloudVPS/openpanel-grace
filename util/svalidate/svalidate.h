#ifndef _svalidate_H
#define _svalidate_H 1
#include <grace/application.h>

//  -------------------------------------------------------------------------
/// Main application class.
//  -------------------------------------------------------------------------
class svalidateApp : public application
{
public:
		 	 svalidateApp (void) :
				application ("nl.madscience.grace.tools.svalidate")
			 {
			 }
			~svalidateApp (void)
			 {
			 }

	int		 main (void);
};

#endif
