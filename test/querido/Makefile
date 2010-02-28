include ../../src/libquerido/makeinclude

OBJ	= main.o

all: querido.exe
	mkapp querido

querido.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o querido.exe $(OBJ) -L../../lib -lgrace -lquerido $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf querido.app
	rm -f querido

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
