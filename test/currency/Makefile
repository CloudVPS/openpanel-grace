include ../../src/libgrace/makeinclude

OBJ		= main.o

all: currency.exe
		mkapp currency

currency.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o currency.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf currency.app
		rm -f currency

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

