include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_byint.exe
		mkapp value_byint

value_byint.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_byint.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_byint.app
		rm -f value_byint

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

