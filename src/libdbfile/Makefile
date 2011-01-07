# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, using version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

include makeinclude

OBJ			= 	dbfile.o gdbmfile.o db4file.o

AR = ar cr

all: $(OBJ)
	$(LDSHARED) -o ../../lib/libdbfile.$(DYNEXT) $(OBJ)
	$(AR) ../../lib/libdbfile.a $(OBJ)

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
