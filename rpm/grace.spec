Name: 		grace
Version: 	0.9.38
Release: 	1%{?dist}
Summary:  	The grace library
License: 	GPLv3
Packager:	Igmar Palsenberg <igmar@palsenberg.com>
Group: 		System Environment/Libraries
Source: 	%{name}-%{version}.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Provides:	%{name} = %{version}-%{release}

%description
C++ has gotten a bad rep and attracted a lot of haters. Still, the language is
far from dead and not very likely to die any time soon. Lots of effort has been
put, through various projects, into creating systems and libraries that hide
some of the more ugly parts and ease programming. Grace is one of those
attempts. It is a project that has grown over a couple of years to introduce,
through a combination of useful classes and clever macros, clear and easy ways
to perform a lot of computing tasks in C++.

%package devel
Summary:	Libraries and include files to develop with the grace library
Group:		Development/Libraries
Requires:	grace = %{version}

%description devel
The grace-devel package contains the libraries and include files to develop
applications using the grace library

%package dbfile
Summary: 	Add support for dbm and db4 to the grace library
Group:		System Environment/Libraries
Requires:	grace = %{version}
Requires: 	gdbm >= 1.8, db4 => 4.3.29
BuildRequires: 	gdbm-devel >= 1.8, db4-devel => 4.3.29

%description dbfile
This package adds support for dbm and db4.6 to the grace library

%package dbfile-devel
Summary: 	Add support for dbm and db4 to the grace library
Group: 		System Environment/Libraries
Requires: 	grace-devel = %{version}
Requires: 	gdbm >= 1.8, db4 => 4.3.29
BuildRequires:	gdbm-devel >= 1.8, db4-devel => 4.3.29

%description dbfile-devel
Libraries and include files for the dbfile development with the grace library

%package querido
Summary: 	Add support for dbm and db4 to the grace library
Group:		System Environment/Libraries
Requires: 	grace = %{version}
Requires:	sqlite >= 3.3.6

%description querido
This package adds support for databases to the grace library.

%package querido-devel
Summary: 	Add support for dbm and db4 to the grace library
Group:		Development/Libraries
Requires: 	grace-devel = %{version}
BuildRequires:	sqlite-devel >= 3.3.6

%description querido-devel
Libraries and include files for querido development with the grace library

%package ssl
Summary: 	Add support for SSL to the grace library
Group:		System Environment/Libraries
Requires: 	grace = %{version}
Requires: 	openssl >= 0.9.8

%description ssl
This package adds support for SSL to the grace library.

%package ssl-devel
Summary: 	Add support for dbm and db4 to the grace library
Group:		Development/Libraries
Requires: 	grace-devel = %{version}
BuildRequires: 	openssl-devel >= 0.9.8

%description ssl-devel
Libraries and include files for SSL development with the grace library

%package pcre
Summary: 	Add support for PCRE to the grace library
Group:		System Environment/Libraries
Requires: 	grace = %{version}
Requires: 	pcre >= 6.6

%description pcre
This package adds support for advanced regluar expessions to the grace library 
through PCRE.

%package pcre-devel
Summary: 	Add support for pcre to the grace library
Group:		Development/Libraries
Requires: 	grace-devel = %{version}
BuildRequires: 	pcre-devel >= 6.6

%description pcre-devel
Libraries and include files for PCRE development with the grace library

%prep
%setup -q -n %{name}
./configure --prefix=%{_prefix} --exec-prefix=%{_bindir} \
            --lib-prefix=%{_libdir} --conf-prefix=%{_sysconfdir} \
	    --include-prefix=%{_includedir}

%build
make

%install
rm -rf %{buildroot}
%makeinstall DESTDIR=%{buildroot}
# Wipe file that shouldn't be packaged
rm -f %{buildroot}/%{_libdir}/libmatrixsslstatic.a

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%post dbfile -p /sbin/ldconfig
%postun dbfile -p /sbin/ldconfig

%post querido -p /sbin/ldconfig
%postun querido -p /sbin/ldconfig

%post ssl -p /sbin/ldconfig
%postun ssl -p /sbin/ldconfig

%post pcre -p /sbin/ldconfig
%postun pcre -p /sbin/ldconfig

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_libdir}/libgrace.so.*

%files devel
%defattr(-,root,root)
%{_libdir}/libgrace.so
%{_libdir}/libgrace.a
%{_bindir}/grace
%{_bindir}/mkapp
%{_bindir}/mkapplinks
%{_bindir}/mkconf
%{_bindir}/mkproject
%{_bindir}/mkversion
%{_bindir}/util-grace/
%{_includedir}/grace/
%{_libdir}/grace-configure/

%files dbfile
%defattr(-,root,root)
%{_libdir}/libdbfile.so.*

%files dbfile-devel
%defattr(-,root,root)
%{_libdir}/libdbfile.so
%{_libdir}/libdbfile.a
%{_includedir}/dbfile/


%files querido
%defattr(-,root,root)
%{_libdir}/libquerido.so.*

%files querido-devel
%defattr(-,root,root)
%{_libdir}/libquerido.so
%{_libdir}/libquerido.a
%{_includedir}/querido/

%files ssl
%defattr(-,root,root)
%{_libdir}/libgrace-ssl.so.*

%files ssl-devel
%defattr(-,root,root)
%{_libdir}/libgrace-ssl.so
%{_libdir}/libgrace-ssl.a
%{_includedir}/matrixssl/

%files pcre
%defattr(-,root,root)
%{_libdir}/libgrace-pcre.so.*

%files pcre-devel
%defattr(-,root,root)
%{_libdir}/libgrace-pcre.so
%{_libdir}/libgrace-pcre.a


%changelog
* Tue Jan 18 2011 Igmar Palsenberg <igmar@palsenberg.com>
- Initial RPM packaging
