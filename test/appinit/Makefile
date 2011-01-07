include ../../src/libgrace/makeinclude

OBJ	= main.o

all: appinit.exe
	mkapp appinit

appinit.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o appinit.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf appinit.app
	rm -f appinit

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
