include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_xmlbadform.exe
	mkapp value_xmlbadform

value_xmlbadform.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_xmlbadform.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_xmlbadform.app
	rm -f value_xmlbadform

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
