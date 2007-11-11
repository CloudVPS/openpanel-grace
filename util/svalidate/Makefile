include ../../src/libgrace/makeinclude

OBJ	= main.o

all: svalidate.exe
	cp ../../schemas/grace.root.validator.xml rsrc/
	cp ../../schemas/grace.validator.validator.xml rsrc
	mkapp svalidate

install:
	./makeinstall

svalidate.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o svalidate.exe $(OBJ) $(LIBS) -L../../lib -lgrace

clean:
	rm -f *.o *.exe
	rm -rf svalidate.app
	rm -f svalidate
	rm -f rsrc/grace.*.validator.xml

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(SHARED) $(CXXFLAGS) $(INCLUDES) -I../../include -c $<
