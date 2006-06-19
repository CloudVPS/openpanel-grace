OBJ	= main.o

all: value_byint.exe
	mkapp value_byint

value_byint.exe: $(OBJ)
	$(CXX) -o value_byint.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_byint.app
	rm -f value_byint

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
