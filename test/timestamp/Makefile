include ../../src/libgrace/makeinclude

OBJ		= main.o

all: timestamp.exe
		mkapp timestamp

timestamp.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o timestamp.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf timestamp.app
		rm -f timestamp

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

