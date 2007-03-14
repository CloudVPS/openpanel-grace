include ../../src/libgrace/makeinclude

OBJ	= main.o

all: fswatch.exe
	mkapp fswatch

fswatch.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o fswatch.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf fswatch.app
	rm -f fswatch

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
