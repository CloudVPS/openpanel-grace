# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, using version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

include makeinclude

OBJ			= 	application.o \
				atoll.o \
				cgi.o \
				checksum.o \
				cmdtoken.o \
				commonkeys.o \
				configdb.o \
				currency.o \
				daemon.o \
				defaults.o \
				dictionary.o \
				eventq.o \
				file.o \
				filesystem.o \
				fswatch.o \
				http.o \
				httpd.o \
				httpd_fileshare.o \
				ipaddress.o \
				lock.o \
				md5.o \
				netdb.o \
				process.o \
				reg.o \
				regexpression.o \
				retain.o \
				ringbuffer.o \
				session.o \
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
				timestamp.o \
				tolower.o \
				udpsocket.o \
				valuable.o \
				value.o \
				value_ascii.o \
				value_ini.o \
				value_ip.o \
				value_grace.o \
				value_json.o \
				value_msgpack.o \
				value_php.o \
				value_plist.o \
				value_sort.o \
				value_strformat.o \
				value_xml.o \
				value_csv.o \
				value_cxml.o \
				value_shox.o \
				version.o \
				xmlschema_root.o \
				xmlschema_base.o \
				xmlschema_misc.o \
				xmlschema.o \
				validator.o \
				valueindex.o

AR = ar cr
LIBNAME = libgrace
LIBVERSION = 1.1

all: ../../include/grace/platform.h $(OBJ)
	mkdir -p ../../lib
	$(LDSHARED) $(LIBS) -o ../../lib/$(LIBFILENAME) $(OBJ) $(LIBS)
	$(AR) ../../lib/libgrace.a $(OBJ)

version.cpp:
	../../util/mkversion version.cpp

install:
	./makeinstall

depend:
	@echo -n > Makefile.depend
	@makedepend -Y../../include -fMakefile.depend *.cpp 2>/dev/null

clean:
	rm -f *.o *.a version.cpp

../../include/grace/platform.h: platform.h
	cp -f platform.h ../../include/grace/platform.h

.SUFFIXES: .cpp .o
.cpp.o: 
	$(CXX) $(SHARED) $(CXXFLAGS) $(INCLUDES) -I../../include -c -g $<

include Makefile.depend
# DO NOT DELETE
