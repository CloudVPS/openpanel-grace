include ../../src/libgrace/makeinclude

OBJ		= main.o

all: application.exe
		mkapp application

application.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o application.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf application.app
		rm -f application

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

