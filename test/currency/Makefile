OBJ	= main.o

all: currency.exe
	mkapp currency

currency.exe: $(OBJ)
	$(CXX) -o currency.exe $(OBJ) "../../lib/libgrace.a" -lpthread

clean:
	rm -f *.o *.exe
	rm -rf currency.app
	rm -f currency

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -I"../../include" -g -c $<
