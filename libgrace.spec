# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, using version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

%define version 0.9.0

%define libpath /usr/lib
%ifarch x86_64
  %define libpath /usr/lib64
%endif

%define tag gen
%define is_fedora %(test -e /etc/fedora-release && echo 1 || echo 0)
%if %is_fedora
  %define fedora_release %(rpm -q --queryformat '%{VERSION}' fedora-release)
  %define fedora_version %(echo "%fedora_release" | tr -d '.')
  %define tag fc%fedora_version
%endif
%define is_centos %(grep -q CentOS /etc/redhat-release && echo 1 || echo 0)
%if %is_centos
  %define centos_release %(rpm -q --queryformat '%{VERSION}' centos-release)
  %define centos_version %(echo "%centos_release" | tr -d '.')
  %define tag EL%centos_version
%endif

Summary: Grace C++ toolkit library
Name: libgrace
Version: %version
Release: 1.%tag
License: LGPL
Group: System Environment/Libraries
Source: http://packages.openpanel.com/archive/grace-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

%description
The Grace C++ library

%package devel
Summary: Header files for the Grace C++ toolkit library
Group: System Environmnent/Libraries

%description devel
Headers for building Grace software.

%package ssl
Summary: Library extension to use SSL communication with Grace.
Group: System Environment/Libraries

%description ssl
Encapsulation of the matrixssl library within the Grace tcpsocket and
httpsocket classes.

%package dbfile
Summary: Grace support library for handling database file formats.
Group: System Environment/Libraries

%description dbfile
Classes for dealing with BerkeleyDB v4 and GDBM files sanely.

%package dbfile-devel
Summary: Header files for the dbfile library.
Group: System Environment/Libraries

%description dbfile-devel
Header classes for dbfile, db4file and gdbmfile.

%package querido
Summary: SQL database access classes for Grace.
Group: System Environment/Libraries

%description querido
Implements access to sqlite databases, with powerful query building and
result access.

%package querido-devel
Summary: Header files for libquerido.
Group: System Environments/Libraries

%description querido-devel
Headers.

%prep
%setup -q -n libgrace-%version

%build
cd src/libgrace && ./configure --prefix /usr --lib-prefix %{libpath}
make
cd ../matrixssl
./configure
make
cd ../libgrace-ssl
./configure --prefix /usr --lib-prefix %{libpath}
make
cd ../libdbfile
./configure --prefix /usr --lib-prefix %{libpath}
make
cd ../libquerido
./configure --prefix /usr --lib-prefix %{libpath}
make
cd ../..

%install
mkdir -p $RPM_BUILD_ROOT%libpath
install -m 644 lib/libgrace.so $RPM_BUILD_ROOT%{libpath}/
install -m 644 lib/libgrace-ssl.so $RPM_BUILD_ROOT%{libpath}/
install -m 644 lib/libdbfile.so $RPM_BUILD_ROOT%{libpath}/
install -m 644 lib/libquerido.so $RPM_BUILD_ROOT%{libpath}/
mkdir -p $RPM_BUILD_ROOT/usr/include/grace
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
%{libpath}/libgrace.so
%files devel
/usr/include/grace
%files ssl
%{libpath}/libgrace-ssl.so
%files dbfile
%{libpath}/libdbfile.so
%files dbfile-devel
/usr/include/dbfile
%files querido
%{libpath}/libquerido.so
%files querido-devel
/usr/include/querido

