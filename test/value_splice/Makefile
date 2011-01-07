include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_splice.exe
	mkapp value_splice

value_splice.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_splice.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_splice.app
	rm -f value_splice

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
