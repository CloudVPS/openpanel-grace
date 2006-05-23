OBJ	= main.o

all: threadgroup.exe
	mkapp threadgroup

threadgroup.exe: $(OBJ)
	$(CXX) -o threadgroup.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf threadgroup.app
	rm -f threadgroup

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
