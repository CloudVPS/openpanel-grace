OBJ	= main.o

all: value_shox.exe
	mkapp value_shox

value_shox.exe: $(OBJ)
	$(CXX) -o value_shox.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_shox.app
	rm -f value_shox

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
