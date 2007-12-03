include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_json.exe
	mkapp value_json

value_json.exe: $(OBJ)
	$(CXX) $(LDFLAGS) -o value_json.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf value_json.app
	rm -f value_json

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
