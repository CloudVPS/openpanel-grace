include ../../src/libgrace/makeinclude

OBJ		= main.o

all: string.exe
		mkapp string

string.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o string.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf string.app
		rm -f string

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

