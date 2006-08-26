include ../../src/libdbfile/makeinclude

OBJ	= main.o

all: gdbm.exe
	mkapp gdbm

gdbm.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o gdbm.exe $(OBJ) "../../lib/libdbfile.a" \
	"../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf gdbm.app
	rm -f gdbm

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I"../../include" -c $<
