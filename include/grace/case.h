// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _CASE_H
#define _CASE_H 1

#define caseselector(varname) \
	for (bool __case_flip=true; __case_flip; __case_flip=false) \
	for (typeof ( varname ) &__case_ref = varname ; __case_flip; __case_flip=false) {

#define incaseof(valuedef) \
	} for (;__case_flip && __case_ref == valuedef ;__case_flip=false) switch (true) { case true

#define defaultcase \
	} if (! __case_flip) break; switch (true) default

#endif
