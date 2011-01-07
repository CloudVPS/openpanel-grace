include ../../src/libgrace/makeinclude

OBJ		= main.o

all: regexp.exe
		mkapp regexp

regexp.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o regexp.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf regexp.app
		rm -f regexp

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

