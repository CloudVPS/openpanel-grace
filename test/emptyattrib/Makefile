include ../../src/libgrace/makeinclude

OBJ	= main.o

all: emptyattrib.exe
	mkapp emptyattrib

emptyattrib.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o emptyattrib.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf emptyattrib.app
	rm -f emptyattrib

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
