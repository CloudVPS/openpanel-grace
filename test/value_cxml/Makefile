OBJ	= main.o

all: value_cxml.exe
	mkapp value_cxml

value_cxml.exe: $(OBJ)
	$(CXX) -o value_cxml.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_cxml.app
	rm -f value_cxml

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
