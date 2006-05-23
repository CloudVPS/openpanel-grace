OBJ	= main.o

all: string.exe
	mkapp string

string.exe: $(OBJ)
	$(CXX) -o string.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf string.app
	rm -f string

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
