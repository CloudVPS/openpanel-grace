include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_xmlimplied.exe
		mkapp value_xmlimplied

value_xmlimplied.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_xmlimplied.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_xmlimplied.app
		rm -f value_xmlimplied

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

