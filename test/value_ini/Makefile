include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_ini.exe
		mkapp value_ini

value_ini.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_ini.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_ini.app
		rm -f value_ini

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

