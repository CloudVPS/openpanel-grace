OBJ	= main.o

all: value_xmlgremlins.exe
	mkapp value_xmlgremlins

value_xmlgremlins.exe: $(OBJ)
	$(CXX) -o value_xmlgremlins.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_xmlgremlins.app
	rm -f value_xmlgremlins

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
