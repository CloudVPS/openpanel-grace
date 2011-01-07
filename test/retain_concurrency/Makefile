include ../../src/libgrace/makeinclude

OBJ	= main.o

all: retain_concurrency.exe
	mkapp retain_concurrency

retain_concurrency.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o retain_concurrency.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf retain_concurrency.app
	rm -f retain_concurrency

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
