OBJ	= main.o

all: value_xmlbase64.exe
	mkapp value_xmlbase64

value_xmlbase64.exe: $(OBJ)
	$(CXX) -o value_xmlbase64.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_xmlbase64.app
	rm -f value_xmlbase64

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
