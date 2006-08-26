include makeinclude

OBJ			= 	dbfile.o gdbmfile.o

AR = ar cr

all: $(OBJ)
	$(AR) ../../lib/libdbfile.a $(OBJ)
	$(RANLIB) ../../lib/libdbfile.a

install: all
	./makeinstall

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include -I.. -g -c $<
