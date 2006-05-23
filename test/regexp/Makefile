OBJ	= main.o

all: regexp.exe
	mkapp regexp

regexp.exe: $(OBJ)
	$(CXX) -o regexp.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf regexp.app
	rm -f regexp

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
