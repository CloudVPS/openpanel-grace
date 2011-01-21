include ../../src/libgrace-pcre/makeinclude

OBJ	= main.o

all: pcre.exe
	mkapp pcre

pcre.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o pcre.exe $(OBJ) ../../lib/libgrace-pcre.a ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf pcre.app
	rm -f pcre

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I"../../include" -c $<
