OBJ	= main.o

all: value_xmlwrap.exe
	mkapp value_xmlwrap

value_xmlwrap.exe: $(OBJ)
	$(CXX) -o value_xmlwrap.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_xmlwrap.app
	rm -f value_xmlwrap

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
