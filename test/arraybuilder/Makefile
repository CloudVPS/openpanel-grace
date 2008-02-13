include ../../src/libgrace/makeinclude

OBJ	= main.o

all: arraybuilder.exe
	mkapp arraybuilder

arraybuilder.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o arraybuilder.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf arraybuilder.app
	rm -f arraybuilder

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
