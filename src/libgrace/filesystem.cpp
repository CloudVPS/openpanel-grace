// ========================================================================
// filesystem.cpp: Filesystem abstraction class with alias paths.
//
// (C) Copyright 2004-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/filesystem.h>

// Evil unix includes ;)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <grace/system.h>

extern char **environ;

// ========================================================================
// CREATOR ::filesystem
// --------------------
// The filesystem object is normally accessed through its global instance
// that is conveniently named "fs". This constructor is called during
// initialization of the application. It parses the UNIX environment
// variables looking for those matching the profile "PATH_FOO". Those
// that are found are integrated as path volumes. To have a volume
// available as "quux:", define something like PATH_QUUX=/foo/bar:/baz.
//
// The regular UNIX PATH is put into a path volume called "tools:".
// The user's home directory is installed as "home:".
// The directory containing user home directories, if it can be found,
// is available as "home:".
// The "library:" path volume, if not defined, will point to the
// combination of "homes:Shared/Library" and "home:Library". 
// ========================================================================
filesystem::filesystem (void)
{
	signal (SIGPIPE, SIG_IGN);
	umask (0022);
	
	int c=0;
	value nv;
	value v;
	string nam;
	
	_groupcnt = getgroups (0, NULL);
	_groups = new gid_t[_groupcnt+1];
	getgroups (_groupcnt, _groups);
	_groups[_groupcnt++] = getgid();
	
	while (environ[c])
	{
		if (strncmp (environ[c], "_PATH_", 6) == 0)
		{
			string v;
			nv = strutil::split (environ[c], '=');
			v = nv[0].sval();
			nam = v.mid (6);
			nam.ctolower();
			
			pathvol[nam] = strutil::split (nv[1], ':');
		}
		else if (strncmp (environ[c], "PATH=", 5) == 0)
		{
			nv = strutil::split (environ[c], '=');
			pathvol["tools"] = strutil::split (nv[1], ':');
		}
		++c;
	}
	
	string hom;
	hom = ::getenv("HOME");
	pathvol["home"].newval() = hom;
	
	if (! pathvol.exists ("homes"))
	{
		if (fs.exists ("/Users"))
			pathvol["homes"].newval() = "/Users";
		else if (fs.exists ("/home"))
			pathvol["homes"].newval() = "/home";
		else if (fs.exists ("/Home"))
			pathvol["homes"].newval() = "/Home";
		else if (fs.exists ("/usr/people"))
			pathvol["homes"].newval() = "/usr/people";
		else pathvol["homes"].newval() = "/";
	}
	
	if (! pathvol.exists ("library"))
	{
		if (fs.exists ("/Library"))
		{
			pathvol["library"].newval() = "/Library";
		}
		if (fs.exists ("/usr/share"))
		{
			pathvol["library"].newval() = "/usr/share";
		}
		if (fs.exists ("/usr/local/share"))
		{
			pathvol["library"].newval() = "/usr/local/share";
		}
		
		string hlib;
		foreach (home, pathvol["homes"])
		{
			hlib = "%s/Shared/Library" %format (home);
			pathvol["library"].newval() = hlib;
		}
		
		hlib = "%s/.library" %format (hom);
		if (! fs.exists (hlib))
		{
			hlib = "%s/Library" %format (hom);
		}
		
		if (fs.exists (hlib))
			pathvol["library"].newval() = hlib;
	}
	
	if (! pathvol.exists ("var"))
	{
		pathvol["var"].newval() = "/var";
		pathvol["log"].newval() = "/var/log";
		pathvol["run"].newval() = "/var/run";

		string tstr = hom;
		tstr.strcat ("/var");
		if (fs.exists (tstr))
		{
			string varpath;
			string logpath;
			string runpath;
			varpath = transw ("home:var");
			pathvol["var"].newval() = varpath;
			logpath = transw ("var:log");
			pathvol["log"].newval() = logpath;
			runpath = transw ("var:run");
			pathvol["run"].newval() = runpath;
		}
	}

	char cwdbuffer[1024];

	_cwd = ::getcwd (cwdbuffer, 1023);
}

// ========================================================================
// DESTRUCTOR ::filesystem
// -----------------------
// No useful work to be done here, let g++ clean up the heap.
// ========================================================================
filesystem::~filesystem (void)
{
}

// ========================================================================
// METHOD ::exists
// ---------------
// Takes a path/filename, translates a path volume prefix if there is any,
// looks in the proper places and reports whether a node by the given
// name exists.
// ========================================================================
bool filesystem::exists (const string &_str)
{
	static struct stat st;
	string str;
	int cpos, spos;
	
	str = pwdize (_str);
	
	cpos = str.strchr (':');
	spos = str.strchr ('/');
	
	if ( (cpos<0) || ((spos>=0)&&(spos<cpos)) )
	{
		if (stat (str.str(), &st) == 0) return true;
	}
	else
	{
		string pvol;
		string fnam;
		bool res;
		
		fnam = str;
		pvol = fnam.cutat (':');
		
		string *s = findread (pvol, fnam);
		if ((*s).strlen()) res = true;
		else res = false;
		delete s;
		return res;
	}
	return false;
}

// ========================================================================
// METHOD ::rm
// -----------
// Remove a file from the filesystem.
// ========================================================================
bool filesystem::rm (const string &_str)
{
	string str;
	
	str = pwdize (_str);
	
	int cpos, spos;
	
	str = pwdize (_str);
	
	cpos = str.strchr (':');
	spos = str.strchr ('/');
	
	if ( (cpos<0) || ((spos>=0)&&(spos<cpos)) )
	{
		if (unlink (str.str()))
			return false;
		return true;
	}
	
	string pvol, fnam, unixpath;
	
	fnam = str;
	pvol = fnam.cutat (':');
	
	unixpath = findread (pvol, fnam);
	if (unixpath.strlen())
	{
		if (unlink (unixpath.str()))
			return false;
		return true;
	}
	
	return false;
}

// ========================================================================
// METHOD ::mkdir
// --------------
// Create a subdirectory. If inside a path volume, it will be created
// at the leftmost possible path element, skipping over those elements
// that require more privileges.
// ========================================================================
bool filesystem::mkdir (const string &_str)
{
	string str;
	
	int cpos, spos;
	
	str = pwdize (_str);
	
	cpos = str.strchr (':');
	spos = str.strchr ('/');
	
	if ( (cpos<0) || ((spos>=0)&&(spos<cpos)) )
	{
		if (::mkdir (str.str(), 0777 - (0777 & _umask)) == 0)
			return true;
	}
	else
	{
		string pvol;
		string fnam;
		string fpath;
		
		fnam = str;
		pvol = fnam.cutat (':');
		
		fpath = findwrite (pvol, fnam);
		if (! fpath.strlen()) return false;
		if (::mkdir (fpath.str(), 0777 - (0777 & _umask)) == 0)
			return true;
	}
	return false;
}

// ========================================================================
// METHOD ::isdir
// --------------
// Reports whether a provided rich path points to a directory node.
// ========================================================================
bool filesystem::isdir (const string &_path)
{
	string path;
	string p;
	
	path = pwdize (_path);
	p = transr (path);
	
	if (p.strlen())
	{
		struct stat st;
		
		if (stat (p.str(), &st)) return false;
		if ((st.st_mode & S_IFMT) == S_IFDIR) return true;
		return false;
	}
	return false;
}

// ========================================================================
// METHOD ::getinfo
// ----------------
// Gets specific information about a file by its alias path.
// ========================================================================
value *filesystem::getinfo (const string &_path)
{
	returnclass (value) result retain;
	string path;
	string p;
	char linkbuf[512];
	int t;
	
	path = pwdize (_path);
	p = transr (path);
	
	if (p.strlen())
	{
		struct stat st;
		string tstr;
		
		if (stat (p.str(), &st)) return &result;
		
		result["path"] = p;
		result["inode"] = (long long) st.st_ino;
		result["nlink"] = (int) st.st_nlink;
		switch (st.st_mode & S_IFMT)
		{
			case S_IFSOCK:
				result["type"] = fsSocket; break;
			case S_IFLNK:
				result["type"] = fsSoftLink;
				if (( t = ::readlink (p.str(), linkbuf, 511))>=0)
				{
					linkbuf[t] = 0;
					result["link"] = linkbuf;
				}
				break;
			case S_IFREG:
				result["type"] = fsFile; break;
			case S_IFBLK:
				result["type"] = fsBlockDevice; break;
			case S_IFCHR:
				result["type"] = fsCharacterDevice; break;
			case S_IFDIR:
				result["type"] = fsDirectory;
				break;
			default:
				result["type"] = fsUnknown; break;
		}
		result["mode"] = (unsigned int) st.st_mode & 07777;
		result["uid"] = (unsigned int) st.st_uid;
		result["gid"] = (unsigned int) st.st_gid;
		
		value tmpv;

		tmpv = kernel.userdb.getpwuid (st.st_uid);
		if (tmpv.exists ("username"))
		{
			result["user"] = tmpv["username"];
		}
		else
		{
			tstr.crop();
			tstr.printf ("#%u", st.st_uid);
			result["user"] = tstr;
		}
			
		tmpv = kernel.userdb.getgrgid (st.st_gid);
		if (tmpv.exists ("groupname"))
		{
			result["group"] = tmpv["groupname"];
		}
		else
		{
			tstr.crop();
			tstr.printf ("#%u", st.st_gid);
			result["group"] = tstr;
		}
		
		result["size"] = (unsigned int) (st.st_size & 0xffffffff);
		result["atime"] = (unsigned int) st.st_atime;
		result["mtime"] = (unsigned int) st.st_mtime;
		result["ctime"] = (unsigned int) st.st_ctime;
		if ((st.st_mode & S_IFMT) == S_IFDIR) p += "/";
		p.printf (":<mime>");
		if ((t = ::readlink (p.str(), linkbuf, 511)) >= 0)
		{
			linkbuf[t] = 0;
			result["mime"] = linkbuf;
			if ((st.st_mode & S_IFMT) == S_IFDIR)
			{
				result["type"] = fsBundle;
			}
		}
	}
	return &result;
}

// ========================================================================
// METHOD ::cdrelative
// -------------------
// Changes to CWD according to relative path rules. If the provided
// path starts with a slash, or a path volume indication, the path
// is processed as absolute.
// ========================================================================
bool filesystem::cdrelative (const string &path)
{
	string cwd_volume;
	string cwd_path;
	value cwd_tree;
	
	value path_tree;
	
	cwd_path = _cwd;
	cwd_volume = cwd_path.cutat (':');
	if (cwd_path.strlen())
		cwd_tree = strutil::split (cwd_path, '/');
	
	path_tree = strutil::split (path, '/');
	
	foreach (element, path_tree)
	{
		string d;
		d = element;
		
		if (d == "..")
		{
			if (cwd_tree.count() == 0)
				return false;
			cwd_tree.rmindex (cwd_tree.count()-1);
		}
		else cwd_tree.newval() = d;
	}
	
	cwd_path.crop(0);
	if (cwd_volume.strlen())
		cwd_path.strcat ("%s:" %format (cwd_volume));
		
	for (int j=0; j<cwd_tree.count(); ++j)
	{
		if (j) cwd_path += "/";
		cwd_path += cwd_tree[j];
	}
		
	if (isdir (cwd_path))
	{
		_cwd = cwd_path;
		return true;
	}
	
	return false;
}

bool filesystem::chown (const string &path, const string &touser)
{
	string resolved;
	resolved = fs.transw (path);
	
	if (! resolved.strlen())
		return false;
	
	value pwdat = kernel.userdb.getpwnam (touser);
	value finf = fs.getinfo (resolved);
	
	if (! pwdat.count()) return false;
	if (! finf.count()) return false;
		
	uid_t uid;
	gid_t gid;
	
	uid = pwdat["uid"];
	gid = finf["gid"];
	
	if (::chown (resolved.str(), uid, gid)) return false;
	return true;
}

bool filesystem::chown (const string &path, const string &usr, const string &gr)
{
	value pwdat = kernel.userdb.getpwnam (usr);
	value grdat = kernel.userdb.getgrnam (gr);
	
	if (! pwdat.count()) return false;
	if (! grdat.count()) return false;
	
	uid_t uid = pwdat["uid"];
	gid_t gid = grdat["gid"];
	
	return chown (path, uid, gid);
}

bool filesystem::chown (const string &path, uid_t userid, gid_t groupid)
{
	string resolved;
	resolved = fs.transw (path);
	
	if (! resolved.strlen())
		return false;
	
	if (::chown (resolved.str(), userid, groupid)) return false;
	
	return true;
}

bool filesystem::cp (const string &from, const string &to)
{
	string pfrom;
	string pto;
	
	pfrom = transr (from);
	pto = transw (to);
	
	if (pfrom == pto) return false;
	
	file fin;
	file fout;
	
	if (! fin.openread (pfrom)) return false;
	if (! fout.openwrite (pto))
	{
		fin.close();
		return false;
	}
	
	string buf;
	
	while (! fin.eof())
	{
		try
		{
			buf = fin.read (8192);
		}
		catch (exception e)
		{
			break;
		}
		
		if (buf.strlen())
		{
			try
			{
				if (! fout.puts (buf))
				{
					fin.close();
					fout.close();
					fs.rm (pto);
					return false;
				}
			}
			catch (exception e)
			{
				fin.close();
				fout.close();
				fs.rm (pto);
				return false;
			}
		}
		else break;
	}
	fin.close ();
	fout.close ();
	return true;
}

// ========================================================================
// METHOD ::chmod
// ========================================================================
bool filesystem::chmod (const string &path, int perms)
{
	string resolved;
	resolved = fs.transw (path);
	
	if (::chmod (resolved.str(), perms)) return false;
	return true;
}

// ========================================================================
// METHOD ::chgrp
// ========================================================================
bool filesystem::chgrp (const string &path, const string &gr)
{
	string resolved;
	resolved = fs.transw (path);
	
	if (! resolved.strlen())
		return false;
	
	value grdat = kernel.userdb.getgrnam (gr);
	value finf = fs.getinfo (resolved);
	
	if (! grdat.count()) return false;
	if (! finf.count()) return false;
	
	gid_t gid = grdat["gid"];
	uid_t uid = finf["uid"];
	
	if (::chown (resolved.str(), uid, gid)) return false;
	return true;
}

// ========================================================================
// METHOD ::cd
// -----------
// Changes the current directory to a newly provided path.
// ========================================================================
bool filesystem::cd (const string &pat)
{
	int colpos,slashpos;
	int ptype;
	
	enum {
		relativePath,
		absolutePath,
		volumePath
	};
	
	ptype = relativePath;
	
	slashpos = pat.strchr ('/');
	colpos = pat.strchr (':');
	
	if (colpos < 0)
	{
		if (slashpos < 0)
		{
			ptype = relativePath;
		}
		else
		{
			if (slashpos == 0) ptype = absolutePath;
			else ptype = relativePath;
		}
	}
	else
	{
		if (slashpos < 0)
		{
			ptype = volumePath;
		}
		else
		{
			if (slashpos < colpos)
				ptype = slashpos ? relativePath : absolutePath;
			else
				ptype = volumePath;
		}
	}
	
	switch (ptype)
	{
		case relativePath:
			return cdrelative (pat);
		
		case absolutePath:
		case volumePath:
			if (isdir (pat))
			{
				_cwd = pat;
				return true;
			}
			return false;
	}
	
	return false;
}

// ========================================================================
// METHOD ::chroot
// ---------------
// Attempts to chroot to a provided path.
// ========================================================================
bool filesystem::chroot (const string &pat)
{
	if (::chroot (pat.str())) return false;
	return true;
}

// ========================================================================
// METHOD ::mv
// -----------
// Move or rename a file.
// ========================================================================
bool filesystem::mv (const string &_src, const string &_dst)
{
	string src, dst;
	string rsrc, rdst;
	
	src = pwdize (_src);
	dst = pwdize (_dst);
	
	int cpos, spos;
	
	cpos = src.strchr (':');
	spos = src.strchr ('/');
	
	if ( (cpos<0) || ((spos>=0)&&(spos<cpos)) ) rsrc = src;
	else
	{
		string vol, fnam;
		
		fnam = src;
		vol  = fnam.cutat (':');
		rsrc = findwrite (vol, fnam);
		if (! rsrc.strlen()) return false;
	}
	
	cpos = dst.strchr (':');
	spos = dst.strchr ('/');
	
	if ( (cpos<0) || ((spos>=0)&&(spos<cpos)) ) rdst = dst;
	else
	{
		string vol, fnam;
		
		fnam = dst;
		vol  = fnam.cutat (':');
		rdst = findwrite (vol, fnam);
		if (! rdst.strlen()) return false;
	}
	
	return (! rename (rsrc, rdst));
}

// ========================================================================
// METHOD ::umask
// --------------
// Set the local umask for this object (NB unix umask not affected)
// ========================================================================
void filesystem::umask (int numask)
{
	_umask = numask;
}

// ========================================================================
// METHOD ::maywrite
// -----------------
// Determines if we have write access to a provided UNIX path
// ========================================================================
bool filesystem::maywrite (const string &path)
{
	struct stat st;
	uid_t me = geteuid();
	
	if (stat (path, &st))
	{
		// appearantly this path could not be found
		return false;
	}
	if (me == st.st_uid)
	{
		if (st.st_mode & S_IWUSR) return true;
		return false;
	}
	if (st.st_mode & S_IWGRP)
	{
		for (int i=0; i<_groupcnt; ++i)
		{
			if (st.st_gid == _groups[i])
			{
				return true;
			}
		}
	}
	if (st.st_mode & S_IWOTH) return true;
	return false;
}

// ========================================================================
// METHOD ::mayread
// ----------------
// Determines if we have read access to a provided UNIX path
// ========================================================================

bool filesystem::mayread (const string &path)
{
	struct stat st;
	uid_t me = geteuid();
	
	if (stat (path, &st))
	{
		// appearantly this path could not be found
		return false;
	}
	if (me == st.st_uid)
	{
		if (st.st_mode & S_IRUSR) return true;
		return false;
	}
	if (st.st_mode & S_IRGRP)
	{
		for (int i=0; i<_groupcnt; ++i)
		{
			if (st.st_gid == _groups[i])
			{
				return true;
			}
		}
	}
	if (st.st_mode & S_IROTH) return true;
	return false;
}

// ========================================================================
// METHOD ::dir
// ------------
// Returns a brief file listing of a provided path
// ========================================================================
value *filesystem::dir (const string &_path)
{
	return ls (_path, false);
}

// ========================================================================
// METHOD ::ls
// -----------
// Returns a list of file for a provided path. If the path is part of
// a path volume, all parts of that volume are included in the listing.
//
// The filename of each directory entry is used as a key inside the
// value object. In the short format, each key is a dict with one
// child, a string called "path" containing the absolute path of the
// file.
//
// In the long format, this is complemented with the following fields:
//
// inode		The object's inode
// nlink		The number of links to the inode
// type			The basic filetype (fsFile, fsDirectory, fsSocket, etc)
// link			The destination of a softlink (type=fsSoftLink)
// mode			The unix mode permissions
// fuid			The owner uid
// fgid			The owner gid
// size			The file size in bytes
// atime		Time last accessed (unix timestamp)
// mtime		Time last modified (unix timestamp)
// ctime		Time created (unix timestamp)
// mime			The file's mime-type (if available from the filesystem)
// ========================================================================
value *filesystem::ls (const string &_path, bool longformat, bool showhidden)
{
	returnclass (value) res retain;
	string path;
	string suffix;
	value paths;
	char linkbuf[512];
	int t;
	
	if (_path) path = _path;
	else path = pwd();
	
	path = pwdize (path);
	
	int cpos, spos;
	cpos = path.strchr (':');
	spos = path.strchr ('/');
	
	if ( (cpos<0) || ((spos>=0)&&(spos<cpos)) )
	{
		paths.newval() = path;
	}
	else
	{
		value tmp;
		
		tmp = strutil::split (path, ':');
		paths = getpaths (tmp[0]);
		suffix = tmp[1];
	}
	
	foreach (e, paths)
	{
		struct dirent *dir;
		struct stat st;
		DIR *d;
		
		if (suffix.strlen())
			path = "%s/%s" %format (e, suffix);
		else
			path = e;
				
		d = ::opendir(path.str());
		if (d)
		{
			while ((dir = readdir(d)))
			{
				string nam,fpath;
				nam = dir->d_name;
				
				if ((nam[0] == '.') && (!showhidden)) continue;
				if ((nam.strstr (":<mime>") >= 0) && (!showhidden)) continue;
				
				/*if (paths.count()>1)*/
					fpath.printf ("%s/%s", path.str(), nam.str());
				/*else
					fpath = nam;*/
								
				if (! res.exists(nam))
					res[nam]["path"] = fpath;
				else
				{
					string tp = "%s,%s" %format (res[nam]["path"], fpath);
					res[nam]["path"] = tp;
				}
				
				if (longformat)
				{
					string nam;
					string pad;
					nam = dir->d_name;
					
					pad = "%s/%s" %format (path, nam);
					if (! lstat (pad, &st))
					{
						res[nam]["inode"] = (long long) st.st_ino;
						res[nam]["nlink"] = (int) st.st_nlink;
						switch (st.st_mode & S_IFMT)
						{
							case S_IFSOCK:
								res[nam]["type"] = fsSocket; break;
							case S_IFLNK:
								res[nam]["type"] = fsSoftLink;
								if (( t = ::readlink (pad.str(), linkbuf, 511))>=0)
								{
									linkbuf[t] = 0;
									res[nam]["link"] = linkbuf;
								}
								break;
							case S_IFREG:
								res[nam]["type"] = fsFile; break;
							case S_IFBLK:
								res[nam]["type"] = fsBlockDevice; break;
							case S_IFCHR:
								res[nam]["type"] = fsCharacterDevice; break;
							case S_IFDIR:
								res[nam]["type"] = fsDirectory;
								break;
							default:
								res[nam]["type"] = fsUnknown; break;
						}
						res[nam]["mode"] = (unsigned int) st.st_mode & 07777;
						res[nam]["fuid"] = (unsigned int) st.st_uid;
						res[nam]["fgid"] = (unsigned int) st.st_gid;
						res[nam]["size"] = (unsigned int) (st.st_size & 0xffffffff);
						res[nam]["atime"] = (unsigned int) st.st_atime;
						res[nam]["mtime"] = (unsigned int) st.st_mtime;
						res[nam]["ctime"] = (unsigned int) st.st_ctime;
						
						if ((st.st_mode & S_IFMT) == S_IFDIR) pad += "/";
						pad.printf (":<mime>");
						
						if ((t = ::readlink (pad.str(), linkbuf, 511)) >= 0)
						{
							linkbuf[t] = 0;
							res[nam]["mime"] = linkbuf;
							
							if ((st.st_mode & S_IFMT) == S_IFDIR)
							{
								res[nam]["type"] = fsBundle;
							}
						}
					}
				}
			}
			closedir (d);
		}
		else
		{
			res("error") = ::strerror (errno);
		}
	}
	return &res;
}

// ========================================================================
// METHOD ::getresource
// --------------------
// Tries to open a resource file, associated with a provided path.
// Expects a mandatory resource type and an optional index string.
// If the path points to a directory, this method will look for
//
// 		/the/path/.:<restype>
//
// if it is a file it will look for
//
// 		/the/path/.filename:<restype>
//
// If there is no index and the node found so far is not a directory,
// it will be used. If it _is_ a directory, an index of "data" is assumed.
// If there is an index, it will be appended to the path, i.e.:
//
// 		/the/path/.:<restype>/myindex
//		/the/path/.:<restype>:myindex
//		/the/path/.filename:<restype>:myindex
//
// TODO: add more sanity checks here.
// ========================================================================
string *filesystem::getresource (const string &p, const string &rsrc, const string &idx)
{
	returnclass (string) res retain;
	
	string pad;
	string pp;
	struct stat st;
	struct stat st2;
	char linkbuf[512];
	
	pad = pp = p;

	int cpos, spos;
	cpos = pp.strchr (':');
	spos = pp.strchr ('/');
	
	if ( (cpos>0) && ((spos<0)||(spos>cpos)) )
	{
		string vol;
		vol = pp.cutat (':');
		
		pad = findread (vol.str(), pp.str());
	}
	
	if (lstat (pad, &st))
	{
		return &res;
	}
	
	if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		pad += "/.";
	}
	else
	{
		pad = ".%s" %format (pad);
	}
	
	pad.strcat (":<%s>" %format (rsrc));
	
	if (lstat (pad, &st2))
	{
		return &res;
	}
	
	if ((st2.st_mode & S_IFMT) == S_IFDIR)
	{
		pad.strcat ("/%s" %format (idx.strlen() ? idx.str() : "data"));
		if (lstat (pad, &st2)) return &res;
	}
	else
	{
		if (idx.strlen())
		{
			pad.strcat (":%s" %format (idx));
			if (lstat (pad, &st2)) return &res;
		}
	}
		
	if ((st2.st_mode & S_IFMT) == S_IFLNK)
	{
		int t;
		if ((t = ::readlink (pad.str(), linkbuf, 511)) >= 0)
		{
			if (t>511) t = 511;
			linkbuf[t] = 0;
			
			res = linkbuf;
		}
	}
	else if ((st2.st_mode & S_IFMT) == S_IFREG)
	{
		res = fs.load (pad);
	}
	
	return &res;
}

// ========================================================================
// METHOD ::size
// -------------
// Report 32-bit size of a file in the filesystem. Accepts alias paths.
// TODO: Change to 64 bits, even POSIX caught up with this shit.
// ========================================================================
unsigned int filesystem::size (const string &path)
{
	string rpath;
	rpath = transr (path);
	struct stat st;
	if (lstat (rpath.str(), &st)) return 0;
	return (unsigned int) (st.st_size & 0xffffffff);
}

// ========================================================================
// METHOD ::findread
// -----------------
// Resolves a path inside a pathvolume to the rightmost volume part that
// has the named file and allows the user read access to it.
// ========================================================================
string *filesystem::findread (const statstring &pvol, const string &filename)
{
	returnclass (string) resolved retain;
	value res;
	
	res = getpaths (pvol);
	
	for (int i=res.count()-1; i>=0; --i)
	{
		resolved = res[i];
		resolved.strcat ("/%s" %format (filename));
		if (mayread (resolved.str()))
		{
			return &resolved;
		}
	}
	resolved.crop(0);
	return &resolved;
}

// ========================================================================
// METHOD ::getpaths
// -----------------
// Resolves a named path volume to its individual path elements. If the
// volume does not exist, one is created on the fly using:
//
// /volume
// /usr/volume
// /usr/local/volume
// home:/.volume
// ========================================================================

value *filesystem::getpaths (const statstring &pvol)
{
	returnclass (value) res retain;
	
	if (pathvol.exists (pvol)) res = pathvol[pvol];
	else
	{
		res[0] = "/%s" %format (pvol);
		res[1] = "/usr/%s" %format (pvol);
		res[2] = "/usr/local/%s" %format (pvol);
		if (getenv ("HOME"))
		{
			res[3] = "%s/.%s" %format (getenv("HOME"), pvol);
		}
	}
	return &res;
}

// ========================================================================
// METHOD ::findwrite
// ------------------
// For a file, find the leftmost element of a path volume that allows
// the user write access to create or write it.
// ========================================================================
string *filesystem::findwrite (const statstring &pvol, const string &filename)
{
	returnclass (string) resolved retain;
	value res;
	
	res = getpaths (pvol);
	
	foreach (e, res)
	{
		if (e("readonly") == false)
		{
			resolved = e;
			if (maywrite (resolved))
			{
				resolved += "/%s" %format (filename);
				if ( (! exists (resolved))||(maywrite (resolved)) )
				{
					return &resolved;
				}
			}
		}
	}
	resolved.crop(0);
	return &resolved;
}

// ========================================================================
// METHOD ::save
// ========================================================================
bool filesystem::save (const string &_vpath, const string &_data,
					   flag::savetype tp)
{
	if (tp == flag::normal) return save (_vpath, _data);
	
	string tmpnam;
	string uuid = strutil::uuid();
	file f;
	
	tmpnam = _vpath;
	tmpnam.strcat (".");
	tmpnam.strcat (uuid);
	
	if (! f.openwrite (tmpnam)) return false;
	if (! f.puts (_data))
	{
		f.close();
		return false;
	}
	f.close();
	if (! fs.mv (tmpnam, _vpath))
	{
		fs.rm (tmpnam);
		return false;
	}
	
	return true;
}


// Global instance
filesystem fs;
