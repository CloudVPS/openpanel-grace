include ../../src/libgrace/makeinclude

OBJ	= main.o

all: fork.exe
	mkapp fork

fork.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o fork.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf fork.app
	rm -f fork

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -g $(CXXFLAGS) -I"../../include" -c $<
