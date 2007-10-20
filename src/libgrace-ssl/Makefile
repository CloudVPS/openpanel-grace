include makeinclude

OBJ			= 	sslsocket.o

AR = ar cr

all: $(OBJ)
	./getmatrixobj.sh
	$(LDSHARED) -o ../../lib/libgrace-ssl.$(DYNEXT) $(OBJ) matrixobj/*.o

install: all
	./makeinstall

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(SHARED) $(CXXFLAGS) $(INCLUDES) -I../../include -I.. -g -c $<
