// ========================================================================
// fswatch.cpp: Crude filesystem watcher
//
// (C) Copyright 2007 Grace Team <grace@openpanel.com>
//                    OpenPanel V.O.F., Rotterdam
// ========================================================================
#include <grace/fswatch.h>

namespace fschange
{
	statstring deleted ("deleted");
	statstring modified ("modified");
	statstring created ("created");
};

// ========================================================================
// CONSTRUCTOR fswatch
// ========================================================================
fswatch::fswatch (const string &path)
{
	attach (path);
}

fswatch::fswatch (void)
{
}

// ========================================================================
// DESTRUCTOR fswatch
// ========================================================================
fswatch::~fswatch (void)
{
}

// ========================================================================
// METHOD ::attach
// ========================================================================
void fswatch::attach (const string &path)
{
	watchpath = path;
	lastround = fs.ls (watchpath);
}

// ========================================================================
// METHOD ::listchanges
// ========================================================================
value *fswatch::listchanges (void)
{
	if (! watchpath)
	{
		throw (nothingToWatchException());
	}
	
	returnclass (value) res retain;
	
	value nw = fs.ls (watchpath);
	
	foreach (nfile, nw)
	{
		if (lastround.exists (nfile.id()))
		{
			if (lastround[nfile.id()]["mtime"] != nfile["mtime"])
			{
				res[fschange::modified][nfile.id()] = nfile;
			}
		}
		else
		{
			res[fschange::created][nfile.id()] = nfile;
		}
	}
	
	foreach (ofile, lastround)
	{
		if (! nw.exists (ofile.id()))
		{
			res[fschange::deleted][ofile.id()] = ofile;
		}
	}
	
	lastround = nw;
	return &res;
}
