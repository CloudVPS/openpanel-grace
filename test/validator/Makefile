include ../../src/libgrace/makeinclude

OBJ		= main.o

all: validator.exe
		mkapp validator

validator.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o validator.exe $(OBJ) ../../lib/libgrace.a  $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf validator.app
		rm -f validator

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

