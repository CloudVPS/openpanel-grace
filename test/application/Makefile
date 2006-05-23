OBJ	= main.o

all: application.exe
	mkapp application

application.exe: $(OBJ)
	$(CXX) -o application.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf application.app
	rm -f application

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -O2 -c $<
