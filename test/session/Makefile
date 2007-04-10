include ../../src/libgrace/makeinclude

OBJ	= main.o

all: session.exe
	mkapp session

session.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o session.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf session.app
	rm -f session

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
