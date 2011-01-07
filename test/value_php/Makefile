include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_php.exe
		mkapp value_php

value_php.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_php.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_php.app
		rm -f value_php

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

