include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_xmlwrap.exe
		mkapp value_xmlwrap

value_xmlwrap.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_xmlwrap.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_xmlwrap.app
		rm -f value_xmlwrap

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

