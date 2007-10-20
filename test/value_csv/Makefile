include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_csv.exe
		mkapp value_csv

value_csv.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_csv.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_csv.app
		rm -f value_csv

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

