// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _VALIDATOR_H
#define _VALIDATOR_H 1

#include <grace/str.h>
#include <grace/value.h>

/// A class for validating tree data.
class validator
{
public:
					 /// Constructor.
					 /// Use this constructor if you want to do a post-
					 /// creation load of the validation schema file.
					 validator (void);
					 
					 /// Constructor.
					 /// Loads a schema file.
					 /// \param fn The file path/name for the validation
					 ///           schema.
				 	 validator (const string &fn);
				 	~validator (void);
	
					 /// Load schema data.
					 /// \param fn The file path/nbame for the validation
					 ///           schema.
	bool			 load (const string &fn);
	
					 /// Check an object against the schema.
					 /// \param data The object to be checked.
					 /// \param errors Errors will be pushed into
					 ///               this array.
					 /// \return true on success.
	bool			 check (const value &data, string &error);

protected:
	bool			 checkobject (const value &obj, const statstring &id,
								  string &error);

	bool			 matchObject (const value &, const value &, string &);
	
	bool			 matchMandatory (const value &, const value &, string &);
	bool			 matchData (const value &, const value &, string &);
	bool			 matchChild (const value &, const value &, string &);
	bool			 matchAttrib (const value &, const value &, string &);
	bool			 matchHasIndex (const value &, const value &, string &);
	bool			 matchType (const value &, const value &, string &);
	bool			 matchClass (const value &, const value &, string &);
	bool			 matchOr (const value &, const value &, string &);
	bool			 matchAnd (const value &, const value &, string &);
	bool			 matchId (const value &, const value &, string &);
	
	bool			 matchDataText (const value &, const string &);
	bool			 matchDataRegexp (const value &, const string &);
	bool			 matchDataLessThan (const value &, int);
	bool			 matchDataGreaterThan (const value &, int);
	bool			 matchDataMinSize (const value &, int);
	bool			 matchDataMaxSize (const value &, int);

	value			 schema;
	statstring		 currentid;
	
	value			 idchain;
	string			*encodeidchain (void);
	void			 makeerror (string &into, int errorcode,
								const string &errortext,
								const string &detail = "");
};

#endif
