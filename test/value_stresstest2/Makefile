OBJ	= main.o

all: value_stresstest2.exe
	mkapp value_stresstest2

value_stresstest2.exe: $(OBJ)
	$(CXX) -o value_stresstest2.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_stresstest2.app
	rm -f value_stresstest2

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
