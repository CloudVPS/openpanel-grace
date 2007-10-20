include ../../src/libgrace/makeinclude

OBJ		= main.o

all: str_binmethods.exe
		mkapp str_binmethods

str_binmethods.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o str_binmethods.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf str_binmethods.app
		rm -f str_binmethods

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

