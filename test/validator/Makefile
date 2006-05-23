OBJ	= main.o

all: validator.exe
	mkapp validator

validator.exe: $(OBJ)
	$(CXX) -o validator.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf validator.app
	rm -f validator

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
