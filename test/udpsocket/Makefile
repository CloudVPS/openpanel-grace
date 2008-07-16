include ../../src/libgrace/makeinclude

OBJ	= main.o

all: udpsocket.exe
	mkapp udpsocket

udpsocket.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o udpsocket.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf udpsocket.app
	rm -f udpsocket

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
