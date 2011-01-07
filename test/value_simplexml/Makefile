include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_simplexml.exe
	mkapp value_simplexml

value_simplexml.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_simplexml.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_simplexml.app
	rm -f value_simplexml

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
