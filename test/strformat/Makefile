include ../../src/libgrace/makeinclude

OBJ	= main.o

all: strformat.exe
	mkapp strformat

strformat.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o strformat.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf strformat.app
	rm -f strformat

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
