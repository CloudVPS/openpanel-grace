OBJ	= main.o

all: str_binmethods.exe
	mkapp str_binmethods

str_binmethods.exe: $(OBJ)
	$(CXX) -o str_binmethods.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf str_binmethods.app
	rm -f str_binmethods

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
