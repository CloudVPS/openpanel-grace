include makeinclude

OBJ			= 	dbconn.o sqlsession.o

AR = ar cr

all: $(OBJ) drivers
	$(AR) ../../lib/libudbc.a $(OBJ)
	$(RANLIB) ../../lib/libudbc.a

drivers: driver_mysql

driver_mysql: ../../lib/udbc_mysql.so

../../lib/udbc_mysql.so:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include $(SHARED) $(RDYNAMIC) \
	-o ../../lib/udbc_mysql.so drivers/driver_mysql.cpp \
	$(LIBS) ../../lib/libgrace.a

install: all
	echo "install not ready"

clean:
	rm -f *.o *.a

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include -c $<
