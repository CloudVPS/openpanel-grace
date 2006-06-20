OBJ	= main.o

all: strutil_uuid.exe
	mkapp strutil_uuid

strutil_uuid.exe: $(OBJ)
	$(CXX) -o strutil_uuid.exe $(OBJ) "../../lib/libgrace.a" -lpthread #-lcrypt

clean:
	rm -f *.o *.exe
	rm -rf strutil_uuid.app
	rm -f strutil_uuid

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
