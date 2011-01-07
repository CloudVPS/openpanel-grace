include ../../src/libgrace/makeinclude

OBJ		= main.o

all: threadgroup.exe
		mkapp threadgroup

threadgroup.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o threadgroup.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf threadgroup.app
		rm -f threadgroup

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

