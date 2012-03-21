# This file is part of the Grace library, ssl addon (libgrace-ssl).
# The Grace library is free software: you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation, using version 3 of the License.
# You should have received a copy of the GNU General Public License along with 
# Grace library. If not, see <http://www.gnu.org/licenses/>.

include makeinclude

OBJ			= 	sslsocket.o httpsd.o mozillacerts.o

AR = ar cr
LIBNAME = libgrace-ssl
LIBVERSION = 1.1

all: $(OBJ)
	./getmatrixobj.sh
	$(LDSHARED) $(LIBS) -o ../../lib/$(LIBFILENAME) $(OBJ) matrixobj/*.o
	$(AR) ../../lib/libgrace-ssl.a $(OBJ) matrixobj/*.o

install: all
	./makeinstall

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(SHARED) $(CXXFLAGS) $(INCLUDES) -I../../include -I.. -g -c $<
	
	

