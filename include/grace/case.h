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
