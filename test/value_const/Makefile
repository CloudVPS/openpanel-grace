include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_const.exe
	mkapp value_const

value_const.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_const.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_const.app
	rm -f value_const

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
