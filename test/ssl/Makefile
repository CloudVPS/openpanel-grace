include ../../src/libgrace/makeinclude

OBJ		= main.o

all: ssl.exe
		mkapp ssl

ssl.exe: $(OBJ) ../../lib/libgrace.a ../../lib/libgrace-ssl.a
		$(CXX) -g -o0 $(LDFLAGS) -o ssl.exe $(OBJ) ../../lib/libgrace-ssl.a ../../lib/libgrace.a $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf ssl.app
		rm -f ssl

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -g -o0 -I"../../include" -c $<

