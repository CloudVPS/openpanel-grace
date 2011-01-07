include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_xmlgremlins.exe
		mkapp value_xmlgremlins

value_xmlgremlins.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_xmlgremlins.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_xmlgremlins.app
		rm -f value_xmlgremlins

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

