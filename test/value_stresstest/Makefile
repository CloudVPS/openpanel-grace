OBJ	= main.o

all: value_stresstest.exe
	mkapp value_stresstest

value_stresstest.exe: $(OBJ)
	$(CXX) -o value_stresstest.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_stresstest.app
	rm -f value_stresstest

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
