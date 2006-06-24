include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_stresstest.exe
		mkapp value_stresstest

value_stresstest.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_stresstest.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_stresstest.app
		rm -f value_stresstest

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

