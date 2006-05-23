OBJ	= main.o

all: value_ascii.exe
	mkapp value_ascii

value_ascii.exe: $(OBJ)
	$(CXX) -o value_ascii.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_ascii.app
	rm -f value_ascii

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
