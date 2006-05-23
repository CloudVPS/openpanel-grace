OBJ	= main.o

all: http.exe
	mkapp http

http.exe: $(OBJ)
	$(CXX) -o http.exe $(OBJ) "$(HOME)/.lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf http.app
	rm -f http

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -g -I"$(HOME)/.include" -g -c $<
