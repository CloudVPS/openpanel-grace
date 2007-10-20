include ../../src/libgrace/makeinclude

OBJ	= main.o

all: retainbug.exe
	mkapp retainbug

retainbug.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o retainbug.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf retainbug.app
	rm -f retainbug

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
