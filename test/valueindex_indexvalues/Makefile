include ../../src/libgrace/makeinclude

OBJ	= main.o

all: valueindex_indexvalues.exe
	mkapp valueindex_indexvalues

valueindex_indexvalues.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o valueindex_indexvalues.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf valueindex_indexvalues.app
	rm -f valueindex_indexvalues

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
