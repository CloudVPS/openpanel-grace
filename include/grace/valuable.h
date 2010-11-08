// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _VALUABLE_H
#define _VALUABLE_H 1

#include <grace/value.h>

/// An abstract class representing an object that can serialize/deserialize
/// itself to/from a value object.
class valuable : public memory::retainable
{
friend class value;
public:
					 /// Default constructor.
					 valuable (void);
					 
					 /// Virtual destructor.
	virtual			~valuable (void);
	
protected:
					 /// Virtual method for the actual serialization
					 /// into a value. Inheriting classes should override
					 /// this to do something useful.
	virtual void	 tovalue (value &into) const;
	
					 /// Virtual method inherited from memory::retainable.
	virtual void	 init (bool first);
};

#endif
