include makeinclude

OBJ			= 	engine.o query.o row.o table.o

AR = ar cr

all: $(OBJ) 
	$(AR) ../../lib/libquerido.a $(OBJ)
	$(RANLIB) ../../lib/libquerido.a

install: all
	echo "install not ready"

clean:
	rm -f *.o *.a

allclean: clean
	rm -f configure.paths makeinclude platform.h

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include -c $<
