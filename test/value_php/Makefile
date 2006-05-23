OBJ	= main.o

all: value_php.exe
	mkapp value_php

value_php.exe: $(OBJ)
	$(CXX) -o value_php.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_php.app
	rm -f value_php

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -O2 -c $<
