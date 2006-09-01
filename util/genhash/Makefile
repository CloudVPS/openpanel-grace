include ../../src/libgrace/makeinclude

OBJ	= main.o

all: genhash.exe
	mkapp genhash

install:
	./makeinstall

genhash.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o genhash.exe $(OBJ) $(LIBS) ../../lib/libgrace.a

clean:
	rm -f *.o *.exe
	rm -rf genhash.app
	rm -f genhash

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include -c $<
