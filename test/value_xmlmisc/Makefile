include ../../src/libgrace/makeinclude

OBJ		= main.o

all: value_xmlmisc.exe
		mkapp value_xmlmisc

value_xmlmisc.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o value_xmlmisc.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf value_xmlmisc.app
		rm -f value_xmlmisc

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

