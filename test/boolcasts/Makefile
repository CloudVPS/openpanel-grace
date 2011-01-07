include ../../src/libgrace/makeinclude

OBJ		= main.o

all: boolcasts.exe
		mkapp boolcasts

boolcasts.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o boolcasts.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf boolcasts.app
		rm -f boolcasts

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

