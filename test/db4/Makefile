include ../../src/libdbfile/makeinclude

OBJ	= main.o

all: db4.exe
	mkapp db4

db4.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o db4.exe $(OBJ) ../../lib/libdbfile.a ../../lib/libgrace.a $(LIBS)

clean:
	rm -f *.o *.exe mydb out.dat
	rm -rf db4.app
	rm -f db4

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I"../../include" -c $<
