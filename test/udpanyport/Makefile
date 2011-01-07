include ../../src/libgrace/makeinclude

OBJ	= main.o

all: udpanyport.exe
	mkapp udpanyport

udpanyport.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o udpanyport.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf udpanyport.app
	rm -f udpanyport

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
