OBJ	= main.o

all: strutil1.exe
	mkapp strutil1

strutil1.exe: $(OBJ)
	$(CXX) -o strutil1.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf strutil1.app
	rm -f strutil1

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -O2 -c $<
