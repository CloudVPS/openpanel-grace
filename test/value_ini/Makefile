OBJ	= main.o

all: value_ini.exe
	mkapp value_ini

value_ini.exe: $(OBJ)
	$(CXX) -o value_ini.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_ini.app
	rm -f value_ini

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -O2 -c $<
