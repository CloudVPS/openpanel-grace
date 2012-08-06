include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_emptystr.exe
	mkapp value_emptystr

value_emptystr.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_emptystr.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_emptystr.app
	rm -f value_emptystr

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
