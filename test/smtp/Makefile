include ../../src/libgrace/makeinclude

OBJ		= main.o

all: smtp.exe
		mkapp smtp

smtp.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o smtp.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf smtp.app
		rm -f smtp

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

