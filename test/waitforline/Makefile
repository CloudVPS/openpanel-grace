OBJ	= main.o

all: waitforline.exe
	mkapp waitforline

waitforline.exe: $(OBJ)
	$(CXX) -o waitforline.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf waitforline.app
	rm -f waitforline

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
