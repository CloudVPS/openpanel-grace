OBJ	= main.o

all: boolcasts.exe
	mkapp boolcasts

boolcasts.exe: $(OBJ)
	$(CXX) -o boolcasts.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf boolcasts.app
	rm -f boolcasts

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
