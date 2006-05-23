OBJ	= main.o

all: value_xmlimplied.exe
	mkapp value_xmlimplied

value_xmlimplied.exe: $(OBJ)
	$(CXX) -o value_xmlimplied.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_xmlimplied.app
	rm -f value_xmlimplied

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
