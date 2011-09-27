include ../../src/libgrace/makeinclude

OBJ	= main.o

all: strgrow.exe
	mkapp strgrow

strgrow.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o strgrow.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf strgrow.app
	rm -f strgrow

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
