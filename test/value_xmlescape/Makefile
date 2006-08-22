include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_xmlescape.exe
	mkapp value_xmlescape

value_xmlescape.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_xmlescape.exe $(OBJ) "../../lib/libgrace.a" $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_xmlescape.app
	rm -f value_xmlescape

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
