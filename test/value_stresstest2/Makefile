include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_stresstest2.exe
		mkapp value_stresstest2

value_stresstest2.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_stresstest2.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_stresstest2.app
		rm -f value_stresstest2

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

