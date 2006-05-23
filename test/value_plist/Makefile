OBJ	= main.o

all: value_plist.exe
	cp -f ../../schemas/com.apple.plist.schema.xml rsrc/
	mkapp value_plist

value_plist.exe: $(OBJ)
	$(CXX) -o value_plist.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_plist.app
	rm -f value_plist

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
