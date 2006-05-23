OBJ	= main.o

all: value_csv.exe
	mkapp value_csv

value_csv.exe: $(OBJ)
	$(CXX) -o value_csv.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_csv.app
	rm -f value_csv

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -O2 -c $<
