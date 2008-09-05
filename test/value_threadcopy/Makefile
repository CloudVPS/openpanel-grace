include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_threadcopy.exe
	mkapp value_threadcopy

value_threadcopy.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_threadcopy.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_threadcopy.app
	rm -f value_threadcopy

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
