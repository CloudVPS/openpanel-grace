include ../../src/libgrace/makeinclude

OBJ		= main.o

all: iocodec.exe
		mkapp iocodec

iocodec.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o iocodec.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf iocodec.app
		rm -f iocodec

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

