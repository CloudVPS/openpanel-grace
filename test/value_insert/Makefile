include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_insert.exe
	mkapp value_insert

value_insert.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_insert.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_insert.app
	rm -f value_insert

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
