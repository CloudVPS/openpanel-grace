include ../../src/libgrace/makeinclude

OBJ	= main.o

all: exclusivesection.exe
	mkapp exclusivesection

exclusivesection.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o exclusivesection.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf exclusivesection.app
	rm -f exclusivesection

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
