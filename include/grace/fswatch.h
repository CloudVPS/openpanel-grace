// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _FSWATCH_H
#define _FSWATCH_H 1

#include <grace/filesystem.h>
#include <grace/statstring.h>
#include <grace/value.h>

$exception (nothingToWatchException, "Nothing to watch");

namespace fschange
{
	extern statstring deleted;
	extern statstring modified;
	extern statstring created;
};

/// A class for watching changes to a directory on the filesystem.
class fswatch
{
public:
						 /// Constructor with explicit path initialization.
						 /// \param path The path to watch
						 /// \param sfx If provided, instead of looking
						 ///            at files in the watchpath proper,
						 ///            each entry is considered a directory
						 ///            that should only be evaluated if it
						 ///            contains a node with this suffix. All
						 ///            timing information is derived from
						 ///            this suffix node (useful for
						 ///            watching a directory full of maildirs).
						 fswatch (const string &path, const string &sfx="");
						 
						 /// Constructor with delayed initialization. Use
						 /// fswatch::attach to initialize.
						 fswatch (void);
		 
		 				 /// Bog standard destructor.
						~fswatch (void);

						 /// Attach the watcher to a specific directory.
	void				 attach (const string &path, const string &sfx="");
	
						 /// Generate a list of changed files. Returns
						 /// a value object with the following keys,
						 /// which contain a list of files, indexed
						 /// by their filename:
						 /// - 'deleted' for files that are no longer there.
						 /// - 'modified' for files that changed
						 /// - 'created' for new files.
	value				*listchanges (void);
	
protected:
	value				 lastround; ///< Cache of last known contents.
	string				 watchpath; ///< The path we're watching.
	string				 suffixpath; ///< Path suffix to apply to analysis
	value				*list (void); ///< List either direct nodes or suffice-nodes.
};

#endif
