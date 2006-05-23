OBJ	= main.o

all: ssl.exe
	mkapp ssl

ssl.exe: $(OBJ)
	$(CXX) -o ssl.exe $(OBJ) "../../lib/libgrace.a" "../../lib/libgrace-ssl.a" "../../src/matrixssl/src/libmatrixsslstatic.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf ssl.app
	rm -f ssl

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I../../src -I"../../include" -O2 -g -c $<
