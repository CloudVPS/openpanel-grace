include makeinclude

OBJ			= 	application.o \
				atoll.o \
				cgi.o \
				checksum.o \
				cmdtoken.o \
				configdb.o \
				currency.o \
				daemon.o \
				dictionary.o \
				file.o \
				filesystem.o \
				http.o \
				httpd.o \
				httpd_fileshare.o \
				lock.o \
				netdb.o \
				process.o \
				reg.o \
				regexpression.o \
				retain.o \
				ringbuffer.o \
				smtp.o \
				smtpd.o \
				socketpool.o \
				statstring.o \
				stringdict.o \
				str.o \
				strutil.o \
				system.o \
				terminal.o \
				tcpsocket.o \
				thread.o \
				timeofday.o \
				timestamp.o \
				tolower.o \
				value.o \
				value_ascii.o \
				value_ini.o \
				value_ip.o \
				value_php.o \
				value_plist.o \
				value_sort.o \
				value_xml.o \
				value_csv.o \
				value_cxml.o \
				value_shox.o \
				xmlschema_root.o \
				xmlschema_base.o \
				xmlschema_misc.o \
				xmlschema.o \
				validator.o \
				valueindex.o

AR = ar cr

all: ../../include/grace/platform.h $(OBJ)
	mkdir -p ../lib
	$(AR) ../../lib/libgrace.a $(OBJ)
	$(RANLIB) ../../lib/libgrace.a

install: all
	./makeinstall

clean:
	rm -f *.o *.a

../../include/grace/platform.h: platform.h
	cp -f platform.h ../../include/grace/platform.h

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../../include -c -g $<
