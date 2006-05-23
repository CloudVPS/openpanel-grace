OBJ	= main.o

all: tcpsocket.exe
	mkapp tcpsocket

tcpsocket.exe: $(OBJ)
	$(CXX) -o tcpsocket.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf tcpsocket.app
	rm -f tcpsocket

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -O2 -g -c $<
