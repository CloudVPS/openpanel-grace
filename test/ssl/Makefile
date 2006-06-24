include ../../src/libgrace/makeinclude

OBJ		= main.o

all: ssl.exe
		mkapp ssl

ssl.exe: $(OBJ)
		$(CXX) $(LDFLAGS) -o ssl.exe $(OBJ) "../../lib/libgrace.a" \
			"../../lib/libgrace-ssl.a" $(LIBS)

clean:
		rm -f *.o *.exe
		rm -rf ssl.app
		rm -f ssl

SUFFIXES: .cpp .o
.cpp.o:
		$(CXX) $(CXXFLAGS) -I"../../include" -c $<

