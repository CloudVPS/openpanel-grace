include ../../src/libgrace/makeinclude

OBJ		= main.o

all: waitforline.exe
		mkapp waitforline

waitforline.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o waitforline.exe $(OBJ) -L../../lib -lgrace $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf waitforline.app
		rm -f waitforline

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

