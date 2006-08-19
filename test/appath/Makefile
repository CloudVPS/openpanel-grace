include ../../src/libgrace/makeinclude

OBJ	= main.o

all: appath.exe
	mkapp appath
	ln -s appath extrasoftlink

appath.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o appath.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf appath.app
	rm -f appath

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
