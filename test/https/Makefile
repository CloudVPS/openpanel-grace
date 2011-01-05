include ../../src/libgrace/makeinclude

OBJ		= main.o

all: https.exe
		mkapp https

https.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o https.exe $(OBJ) ../../lib/libgrace.a ../../lib/libgrace-ssl.a $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf https.app
		rm -f https

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c -g $<

