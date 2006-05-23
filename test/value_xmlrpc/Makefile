OBJ	= main.o

all: value_xmlrpc.exe
	cp -f ../../schemas/org.xmlrpc.schema.xml rsrc/
	mkapp value_xmlrpc

value_xmlrpc.exe: $(OBJ)
	$(CXX) -o value_xmlrpc.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_xmlrpc.app
	rm -f value_xmlrpc

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
