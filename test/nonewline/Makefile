include ../../src/libgrace/makeinclude

OBJ		= main.o

all: nonewline.exe
		mkapp nonewline

nonewline.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o nonewline.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf nonewline.app
		rm -f nonewline

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

