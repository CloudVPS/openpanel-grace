OBJ	= main.o

all: iocodec.exe
	mkapp iocodec

iocodec.exe: $(OBJ)
	$(CXX) -o iocodec.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf iocodec.app
	rm -f iocodec

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<

