include makeinclude

OBJ			= 	dbfile.o gdbmfile.o db4file.o

AR = ar cr

all: $(OBJ)
	$(LDSHARED) -o ../../lib/libdbfile.$(DYNEXT) $(OBJ)

depend:
	@echo -n >Makefile.depend
	@makedepend -Y../../include -fMakefile.depend *.cpp 2>/dev/null

install: all
	./makeinstall

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(SHARED) $(CXXFLAGS) $(INCLUDES) -I../../include -I.. -g -c $<

include Makefile.depend
