include makeinclude

OBJ			= 	dbfile.o gdbmfile.o

AR = ar cr

all: $(OBJ)
	$(AR) ../../lib/libdbfile.a $(OBJ)
	$(RANLIB) ../../lib/libdbfile.a

depend:
	@echo -n >Makefile.depend
	@makedepend -Y../../include -fMakefile.depend *.cpp 2>/dev/null

install: all
	./makeinstall

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include -I.. -g -c $<

include Makefile.depend
