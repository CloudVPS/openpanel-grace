#include <grace/fswatch.h>

namespace fschange
{
	statstring deleted ("deleted");
	statstring modified ("modified");
	statstring created ("created");
};

fswatch::fswatch (const string &path)
{
	attach (path);
}

fswatch::fswatch (void)
{
}

fswatch::~fswatch (void)
{
}

void fswatch::attach (const string &path)
{
	watchpath = path;
	lastround = fs.ls (watchpath);
}

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
