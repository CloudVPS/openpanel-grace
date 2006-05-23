OBJ	= main.o

all: value_xmlmisc.exe
	mkapp value_xmlmisc

value_xmlmisc.exe: $(OBJ)
	$(CXX) -o value_xmlmisc.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf value_xmlmisc.app
	rm -f value_xmlmisc

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
