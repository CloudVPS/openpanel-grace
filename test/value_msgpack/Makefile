include ../../src/libgrace/makeinclude

OBJ	= main.o

all: value_msgpack
	

value_msgpack: $(OBJ) ../../lib/libgrace.a
	$(CXX) $(LDFLAGS) -o value_msgpack $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
	rm -f *.o 
	rm -f value_msgpack

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -I"../../include" -c $<
