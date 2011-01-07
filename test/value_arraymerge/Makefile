include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_arraymerge.exe
	mkapp value_arraymerge

value_arraymerge.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_arraymerge.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_arraymerge.app
	rm -f value_arraymerge

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
