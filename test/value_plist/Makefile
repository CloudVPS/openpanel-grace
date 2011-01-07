include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_plist.exe
		mkapp value_plist

value_plist.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_plist.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_plist.app
		rm -f value_plist

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

