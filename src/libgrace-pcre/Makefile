# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, using version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

include makeinclude

OBJ			= 	pcre.o

AR = ar cr
LIBNAME = libgrace-pcre
LIBVERSION = 1.1

all: $(OBJ)
	$(LDSHARED) $(LIBS) -o ../../lib/$(LIBFILENAME) $(OBJ)
	$(AR) ../../lib/libgrace-pcre.a $(OBJ)

depend:
	@echo -n >Makefile.depend
	@makedepend -Y../../include -fMakefile.depend *.cpp 2>/dev/null

install: all
	./makeinstall

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(SHARED) $(CXXFLAGS) $(INCLUDES) -I../../include -I.. -g -c $<

include Makefile.depend
