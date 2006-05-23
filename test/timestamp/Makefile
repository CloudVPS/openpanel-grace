OBJ	= main.o

all: timestamp.exe
	mkapp timestamp

timestamp.exe: $(OBJ)
	$(CXX) -o timestamp.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf timestamp.app
	rm -f timestamp

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
