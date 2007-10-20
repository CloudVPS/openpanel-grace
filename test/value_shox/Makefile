include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_shox.exe
		mkapp value_shox

value_shox.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_shox.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_shox.app
		rm -f value_shox

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

