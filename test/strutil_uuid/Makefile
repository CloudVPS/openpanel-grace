include ../../src/libgrace/makeinclude

OBJ		= main.o

all: strutil_uuid.exe
		mkapp strutil_uuid

strutil_uuid.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o strutil_uuid.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf strutil_uuid.app
		rm -f strutil_uuid

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

