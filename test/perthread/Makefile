include ../../src/libgrace/makeinclude

OBJ	= main.o

all: perthread.exe
	mkapp perthread

perthread.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o perthread.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf perthread.app
	rm -f perthread

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
