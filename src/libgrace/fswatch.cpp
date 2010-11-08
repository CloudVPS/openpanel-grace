// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// fswatch.cpp: Crude filesystem watcher
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

// ==========================================================================
// METHOD fswatch::list
// ==========================================================================
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
