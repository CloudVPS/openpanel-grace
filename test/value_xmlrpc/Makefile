include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_xmlrpc.exe
		mkapp value_xmlrpc

value_xmlrpc.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_xmlrpc.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_xmlrpc.app
		rm -f value_xmlrpc

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

