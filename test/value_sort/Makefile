OBJ	= main.o

all: value_sort.exe
	mkapp value_sort

value_sort.exe: $(OBJ)
	$(CXX) -o value_sort.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_sort.app
	rm -f value_sort

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -O2 -c $<
