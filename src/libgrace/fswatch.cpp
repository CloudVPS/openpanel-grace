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
fswatch::fswatch (const string &path, const string &sfx)
{
	attach (path, sfx);
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
void fswatch::attach (const string &path, const string &sfx)
{
	watchpath = path;
	suffixpath = sfx;
	lastround = list ();
}

// ========================================================================
// METHOD ::listchanges
// ========================================================================
value *fswatch::listchanges (void)
{
	if (! watchpath)
	{
		throw nothingToWatchException();
	}
	
	returnclass (value) res retain;
	
	value nw = list ();
	
	foreach (nfile, nw)
	{
		if (lastround.exists (nfile.id()))
		{
			if (lastround[nfile.id()]["mtime"] != nfile["mtime"])
			{
				res[fschange::modified][nfile.id()] = nfile;
			}
			if (lastround[nfile.id()]["ctime"] != nfile["ctime"])
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

value *fswatch::list (void)
{
	if (! suffixpath) return fs.ls (watchpath);

	returnclass (value) res retain;
	value dir = fs.dir (watchpath);
	
	foreach (node, dir)
	{
		string withsuffix = node["path"];
		withsuffix.printf ("/%s", suffixpath.str());
		if (fs.exists (withsuffix))
		{
			res[node.id()] = fs.getinfo (withsuffix);
			res[node.id()]["path"] = node["path"];
		}
	}
	
	return &res;
}
