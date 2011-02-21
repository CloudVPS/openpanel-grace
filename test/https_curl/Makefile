include ../../src/libgrace/makeinclude

OBJ		= main.o

all: https

https: $(OBJ)
		$(CXX) $(LDFLAGS) -o https $(OBJ)  ../../lib/libgrace-ssl.a ../../lib/libgrace.a $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf https.app
		rm -f https

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c -g $<

