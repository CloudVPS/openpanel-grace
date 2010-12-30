include ../../src/libgrace/makeinclude

OBJ		= main.o

all: https.exe
		mkapp https

https.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o https.exe $(OBJ) -L../../lib -lgrace -lgrace-ssl $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf https.app
		rm -f https

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c -g $<

