# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

include makeinclude

OBJ			= 	pcre.o

AR = ar cr

all: $(OBJ)
	$(LDSHARED) -o ../../lib/libgrace-pcre.$(DYNEXT) $(OBJ)

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
