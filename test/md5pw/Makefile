include ../../src/libgrace/makeinclude

OBJ	= main.o

all: md5pw.exe
	mkapp md5pw

md5pw.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o md5pw.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf md5pw.app
	rm -f md5pw

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
