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
	virtual void	 tovalue (value &into);
	
					 /// Virtual method inherited from memory::retainable.
	virtual void	 init (bool first);
};

#endif
