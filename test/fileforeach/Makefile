include ../../src/libgrace/makeinclude

OBJ	= main.o

all: fileforeach.exe
	mkapp fileforeach

fileforeach.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o fileforeach.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf fileforeach.app
	rm -f fileforeach

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
