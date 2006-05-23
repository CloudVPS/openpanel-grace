include makeinclude

OBJ			= 	sslsocket.o

AR = ar cr

all: $(OBJ)
	./getmatrixobj.sh
	$(AR) ../../lib/libgrace-ssl.a $(OBJ) matrixobj/*.o
	$(RANLIB) ../../lib/libgrace-ssl.a

install: all
	./makeinstall

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include -I.. -g -c $<
