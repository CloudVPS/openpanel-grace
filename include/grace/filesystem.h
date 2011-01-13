// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H 1

#include <grace/retain.h>
#include <grace/str.h>
#include <grace/strutil.h>
#include <grace/file.h>
#include <grace/flags.h>
#include <grace/perthread.h>

$exception (fileLoadException, "Could not load file");

/// Easy access class to the filesystem.
/// Programs can use a global instance of this class called 'fs'.
/// Automatically resolves alias paths.
class filesystem
{
public:
					 filesystem (void);
					~filesystem (void);
					
	enum loadoptions { mustexist, optional };
	enum filetype { unknown, data, directory,
					chardevice, blockdevice, fifo,
					softlink, socket, bundle };
			
					 /// Check if a filesystem object exists.
					 /// \param path Relative or absolute path to the file.
					 /// \return Status, \b true if the filesystem object exists.
	bool			 exists (const string &path);
	
					 /// Check if a filesystem object is a directory.
					 /// \param path Relative or absolute path to the file.
					 /// \return Status, \b true if the object is a directory.
	bool			 isdir (const string &path);
	
					 /// Remove a filesystem object.
					 /// \param path Relative or absolute path to the file.
					 /// \return Status, \b true if the file was succesfully
					 ///         deleted.
	bool			 rm (const string &path);
	
					 /// Create a directory.
					 /// \param dir Relative or absolute path for the directory.
					 /// \return Status, \b true if the directory was created.
	bool			 mkdir (const string &dir);
	
					 /// Change the filesystem root.
					 /// \param p The filesystem path to chroot to.
					 /// \return Status, \b true if the root was succesfully
					 ///         changed.
	bool			 chroot (const string &p);
	
					 /// Move/rename a filesystem object.
					 /// \param pold The old path.
					 /// \param pnew The new path.
					 /// \return Status, \b true if the object was moved.
	bool			 mv (const string &pold, const string &pnew);
	
					 /// Copy a file on the filesystem.
					 /// \param pold The old path.
					 /// \param pnew The new path.
	bool			 cp (const string &pold, const string &pnew);
	
					 /// Change mode flags for a filesystem object.
					 /// \param path The path to the object.
					 /// \param perms The access permissions.
	bool			 chmod (const string &path, int perms);
	
					 /// Convert a relative or aliaspath to an
					 /// absolute path. 
					 /// \param str The relative/alias path.
					 /// \return New string object with the absolute path.
	string			*pwdize (const string &str);
	
					 /// Set filesystem mask.
					 /// \param msk The mask.
	void			 umask (int msk);
	
					 /// Change object's ownership.
					 /// \param path The grace path.
					 /// \param user The owner's name.
	bool			 chown (const string &path, const string &user);
	
					 /// Change object's ownership and group ownership.
					 /// \param path The grace path.
					 /// \param user The owner's name.
					 /// \param group The group name.
	bool			 chown (const string &path, const string &user,
							const string &group);
						
					 /// Change object's ownership and group ownership.
					 /// \param path The grace path.
					 /// \param userid The unix userid of the new owner.
					 /// \param groupid The unix groupid of the new owner.
	bool			 chown (const string &path, uid_t userid, gid_t groupid);
					
					 /// Change object's group ownership.
					 /// \param path The grace path.
					 /// \param group The group name.
	bool			 chgrp (const string &path, const string &group);
	
					 /// Size up a filesystem object.
					 /// \param path Path to the object.
					 /// \return Size in bytes.
	unsigned int	 size (const string &path);
	
					 /// Get full information about a file.
					 /// The value object contains the following
					 /// keys:
					 ///   \arg path Full path.
					 ///   \arg inode Filesystem inode number.
					 ///   \arg nlink Number of links.
					 ///   \arg type Filetype (see fsfiletype).
					 ///   \arg link In case of a softlink, the link target.
					 ///   \arg mode Permission bits.
					 ///   \arg fuid File owner.
					 ///   \arg fgid File group.
					 ///   \arg user Resolved username for the owner.
					 ///   \arg group Resolved group name for the group.
					 ///   \arg size File size.
					 ///   \arg atime,mtime,ctime Access times.
					 ///   \arg mime (explicit mimetype).
					 /// \param p Path of the file.
					 /// \return New value object.
	value			*getinfo (const string &);
	
					 /// Change current directory.
					 /// \param dir Relative path to the new directory.
					 /// \return Status, \b true if succeeded.
	bool			 cd (const string &dir);
	
					 /// Return a short format directory.
	value			*dir (const string &path = "");
	
					 /// Lists the entries of the current or provided
					 /// directory. The short format returns just
					 /// a named list with nodes only having a 'path'
					 /// key. The long format follows the form of the
					 /// filesystem::info method.
					 /// \param path Path to investigate.
					 /// \param longformat True if long format should be returned.
					 /// \param showhidden True if 'hidden' files should be listed.
					 /// \return New value object.
	value			*ls (const string &path="", bool longformat=true,
						 bool showhidden=false);
						 
					 /// Load a resource object associated with a filesystem
					 /// object.
					 /// \param pat Path to the filesystem object.
					 /// \param rsrc Resource name.
					 /// \param idx Optional resource index.
					 /// \return New string object with the resource data.
	string			*getresource (const string &pat, const string &rsrc,
					 			  const string &idx = "");
					 
					 /// Get current directory path.
					 /// \return Reference to the path variable.
	string			&pwd (void);

					 /// Load a file. Returns a pointer
					 /// to a newly allocated string object.
					 /// \param _vpath Relative or absolute path of the file.
					 /// \return New string object with the file contents.
	string			*load (const string &_vpath, loadoptions o = mustexist);
					 
					 /// Save a string object into a file.
					 /// \param _vpath Relative or absolute path for the file.
					 /// \param _data The string to be saved.
					 /// \return Status, \b true if succesfully saved.
	bool			 save (const string &_vpath, const string &_data);
					 
	bool			 save (const string &p, const string &d,
						   flag::savetype tp);
	
					 /// Translate a relative/aliaspath to open a file for
					 /// reading. Returns a string containing the translated
					 /// absolute Unix path.
					 /// \param _vpath Relative or alias path.
					 /// \return New string object with translated full path.
	inline string	*transr (const string &_vpath)
					 {
					 	returnclass (string) res retain;
						string vpath;
						int pos;
						int pos2;
						 
						 vpath = pwdize (_vpath);
						
						pos2 = vpath.strchr ('/');
						if ((pos = vpath.strchr (':')) < 0) res = vpath;
						else if ((pos2<0) || (pos2 > pos))
						{
							string vol, fil;
							
							vol = vpath.left (pos);
							fil = vpath.mid (pos+1);
							
							res = findread (vol, fil);
						}
						else res = vpath;
						return &res;
					 }
					 
					 /// Translate a relative/aliaspath to open a file for
					 /// writing. Returns a string containing the translated
					 /// absolute Unix path.
					 /// \param _vpath Relative or alias path.
					 /// \return New string object with translated full path.
	string			*transw (const string &_vpath)
					 {
					 	returnclass (string) res retain;
						string vpath;
						int pos;
						int pos2;
						 
						vpath = pwdize (_vpath);
						
						pos2 = vpath.strchr ('/');
						if ((pos = vpath.strchr (':')) < 0) res = vpath;
						else if ((pos2<0) || (pos2 > pos))
						{
							string vol, fil;
							
							vol = vpath.left (pos);
							fil = vpath.mid (pos+1);
							
							res = findwrite (vol, fil);
						}
						else res = vpath;
						return &res;
					 }
	
	value			 pathvol; ///< Database of aliaspath translations.

protected:
	int				 _umask; ///< Filesystem permission mask.
	perthread<string>_cwd; ///< Current directory.
	gid_t			*_groups; ///< Array of groups the user is in.
	int				 _groupcnt; ///< Size of the groups array.
	
	bool			 cdrelative (const string &);
	string			*findread (const statstring &, const string &);
	string			*findwrite (const statstring &, const string &);
	value			*getpaths (const statstring &);
	//value			*findall (const char *, const char *);
	
	bool			 maywrite (const string &);
	bool			 mayread (const string &);
};

/// Global instance.
extern filesystem fs;

#endif
