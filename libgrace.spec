%define version 0.9.0

Summary: Grace C++ toolkit library
Name: libgrace
Version: %version
Release: 0
Copyright: LGPL
Group: System Environment/Libraries
Source: http://packages.openpanel.com/archive/grace-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

%description
The Grace C++ library

%package libgrace-devel
Summary: Header files for the Grace C++ toolkit library
Group: System Environmnent/Libraries

%description libgrace-devel
Headers for building Grace software.

%package libgrace-ssl
Summary: Library extension to use SSL communication with Grace.
Group: System Environment/Libraries

%description libgrace-ssl
Encapsulation of the matrixssl library within the Grace tcpsocket and
httpsocket classes.

%package libgrace-dbfile
Summary: Grace support library for handling database file formats.
Group: System Environment/Libraries

%description libgrace-dbfile
Classes for dealing with BerkeleyDB v4 and GDBM files sanely.

%package libgrace-dbfile-devel
Summary: Header files for the dbfile library.
Group: System Environment/Libraries

%description libgrace-dbfile-devel
Header classes for dbfile, db4file and gdbmfile.

%package libgrace-querido
Summary: SQL database access classes for Grace.
Group: System Environment/Libraries

%description libgrace-querido
Implements access to sqlite databases, with powerful query building and
result access.

%package libgrace-querido-devel
Summary: Header files for libquerido.
Group: System Environments/Libraries

%description libgrace-querido-devel
Headers.

%prep
%setup -q -n libgrace-%version

%build
cd src/libgrace && ./configure --prefix /usr
make
cd ../matrixssl
./configure
make
cd ../libgrace-ssl
./configure --prefix /usr
make
cd ../libdbfile
./configure --prefix /usr
make
cd ../libquerido
./configure --prefix /usr
make
cd ../..

%install
mkdir -p $RPM_BUILD_ROOT/usr/lib
install -m 644 lib/libgrace.so $RPM_BUILD_ROOT/usr/lib/
install -m 644 lib/libgrace-ssl.so $RPM_BUILD_ROOT/usr/lib/
install -m 644 lib/libdbfile.so $RPM_BUILD_ROOT/usr/lib/
install -m 644 lib/libquerido.so $RPM_BUILD_ROOT/usr/lib/
mkdir -p $RPM_BUILD_ROOT/usr/incude/grace
cp include/grace/*.h $RPM_BUILD_ROOT/usr/include/grace/
mkdir -p $RPM_BUILD_ROOT/usr/include/dbfile
cp include/dbfile/*.h $RPM_BUILD_ROOT/usr/include/dbfile/
mkdir -p $RPM_BUILD_ROOT/usr/include/querido
cp include/querido/*.h $RPM_BUILD_ROOT/usr/include/querido

%clean
cd src/libgrace && make clean
cd ../libgrace-ssl && make clean
cd ../libdbfile && make clean
cd ../libquerido && make clean
cd ../..
rm -rf lib/*.so

%files
%files libgrace
/usr/lib/libgrace.so
%files libgrace-devel
/usr/include/grace
%files libgrace-ssl
/usr/lib/libgrace-ssl.so
%files libgrace-dbfile
/usr/lib/libgrace-dbfile.so
%files libgrace-dbfile-devel
/usr/include/dbfile
%files libgrace-querido
/usr/lib/libquerido.so
%files libgrace-querido-devel
/usr/include/querido

