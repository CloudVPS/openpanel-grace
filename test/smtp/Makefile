OBJ	= main.o

all: smtp.exe
	mkapp smtp

smtp.exe: $(OBJ)
	$(CXX) -o smtp.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf smtp.app
	rm -f smtp

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
