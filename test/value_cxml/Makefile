include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_cxml.exe
		mkapp value_cxml

value_cxml.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_cxml.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_cxml.app
		rm -f value_cxml

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

