include ../../src/libgrace/makeinclude

OBJ		= main.o

all: http.exe
		mkapp http

http.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o http.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf http.app
		rm -f http

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

