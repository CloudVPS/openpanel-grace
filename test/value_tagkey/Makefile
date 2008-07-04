include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_tagkey.exe
	mkapp value_tagkey

value_tagkey.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_tagkey.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_tagkey.app
	rm -f value_tagkey

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
