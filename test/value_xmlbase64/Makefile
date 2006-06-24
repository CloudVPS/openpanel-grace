include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_xmlbase64.exe
		mkapp value_xmlbase64

value_xmlbase64.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_xmlbase64.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_xmlbase64.app
		rm -f value_xmlbase64

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

