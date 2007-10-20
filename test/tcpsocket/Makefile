include ../../src/libgrace/makeinclude

OBJ		= main.o

all: tcpsocket.exe
		mkapp tcpsocket

tcpsocket.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o tcpsocket.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf tcpsocket.app
		rm -f tcpsocket

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

