include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_ascii.exe
		mkapp value_ascii

value_ascii.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_ascii.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_ascii.app
		rm -f value_ascii

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

