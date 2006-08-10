include ../../src/libgrace/makeinclude

OBJ	= main.o

all: md5.exe
	mkapp md5

md5.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o md5.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf md5.app
	rm -f md5

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
