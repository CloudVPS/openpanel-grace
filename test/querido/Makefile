include makeinclude

OBJ	= main.o

all: querido.exe
	mkapp querido

querido.exe: $(OBJ) ../../lib/libquerido.a  ../../lib/libgrace.a
	$(CXX) $(LDFLAGS) -lpthread -o querido.exe $(OBJ) ../../lib/libquerido.a  ../../lib/libgrace.a $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf querido.app
	rm -f querido

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
