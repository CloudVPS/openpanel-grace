include ../../src/libgrace/makeinclude

OBJ	= main.o

all: valuable.exe
	mkapp valuable

valuable.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o valuable.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf valuable.app
	rm -f valuable

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
