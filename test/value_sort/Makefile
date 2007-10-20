include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_sort.exe
		mkapp value_sort

value_sort.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_sort.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_sort.app
		rm -f value_sort

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

