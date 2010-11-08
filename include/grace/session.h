// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _SESSION_H
#define _SESSION_H 1

#include <grace/value.h>
#include <grace/lock.h>

/// Represents a session database to be used for keeping tab of state
/// inside stateless protocols. Primarily it's just a collection of
/// value objects indexed by a generated uuid, with an extra 'time' field
/// keeping track of access times, allowing for sessions to expire over time.
/// The design for this sessionlist assumes that there is never a need
/// for parallel access to the same session data but does not perform any
/// magic to make this impossible.
class sessionlist
{
public:
					 /// Creator.
					 sessionlist (void);
					 
					 /// Destructor.
					~sessionlist (void);
					
					 /// Check whether a session is active.
					 /// \param id The session uuid.
					 /// \return True if the session exists.
	bool			 exists (const statstring &id);
	
					 /// Create a new session.
					 /// \param sdat Initial session data.
					 /// \return A retainable string containing the uuid
					 ///         for the newly created session.
	string			*create (const value &sdat);
	
					 /// Remove a session.
					 /// \param id The session uuid.
	void			 destroy (const statstring &id);
	
					 /// Get the data for a specific id. Note that you
					 /// should already have gone through
					 /// sessionlist::exists(), at this point a session
					 /// would be auto-created as empty if there is
					 /// no session with the provided id.
					 /// \param id The session uuid;
					 /// \return Retainable value object with the session-
					 ///         related data.
	value			*get (const statstring &id);
	
					 /// Update session-related data for a specific id.
					 /// Replaces any earlier data.
					 /// \param id The session uuid;
					 /// \param dat The new session data.
	void			 set (const statstring &id, const value &dat);
	
					 /// Remove any session objects that exceed a
					 /// given timeout.
					 /// \param timeout The sessopm timeout in seconds.
	void			 expire (int timeout);
	
protected:
	lock<value>		 db;
};

#endif
