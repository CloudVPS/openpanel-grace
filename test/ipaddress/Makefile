include ../../src/libgrace/makeinclude

OBJ	= main.o

all: ipaddress.exe
	mkapp ipaddress

ipaddress.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o ipaddress.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf ipaddress.app
	rm -f ipaddress

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
