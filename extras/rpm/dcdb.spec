%define _unpackaged_files_terminate_build 0
%define name dcdb
%define version 1.2
%define release 18
Summary: DCDB Database Engine
Name: %{name}
Version: %{version}
Release: %{release} 
Source: %{name}-%{version}-%{release}.tar.gz
Copyright: GPL/Perl Artistic
Group: Development/Databases
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}
BuildRequires: perl
Packager: David May <dmay@tvi.edu>
Requires: tcl >= 8.4
URL: http://w3.tvi.edu/~dmay/dcdb.html


%package doc
Group: Documentation
Requires: tetex-latex
Summary: Latex documentation files
Provides: cdb.dvi sort.dvi


%package lib
Group: System Environment/Libraries
Summary: DCDB Database Engine Libraries

%package extra
Group: System Environment/Libraries
Requires: %{name}-%{version}-%{release}
Summary: DCDB Database Engine Binding Library

%description
DCDB is a database engine similar in scope to CodeBase (R), although 
it is not nearly as feature rich and is not an xBase engine. DCDB is
GPL  software,  or  if  you prefer, the Perl Artistic license.  This 
software  is  released  under  the GPL with the additional exemption 
that compiling,  linking,  and/or using OpenSSL is allowed.   At any 
rate,  DCDB  can  be  freely copied and used in open source projects
which  don't  inhibit  the  use  of  OpenSSL.  If  you  are  using a 
distribution that includes OpenSSL, you should have no worries about
using this software.


%description doc
This package contains the documentation for DCDB.


%description lib
This package contains the libraries and header files for DCDB.

%description extra
This package contains TCL binding library.

%prep
%setup


%build
./configure --prefix="$RPM_BUILD_ROOT/usr"                   && \
make                                                         && \
( cd doc && make && cd .. )                                  && \
( cd lib && make libcdbtcl.so )


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make install
cd lib                                                 && \
make install_libcdbtcl                                 && \
cd ..
cd doc                                                 && \
make install

%clean
rm -rf $RPM_BUILD_ROOT


%files
%attr (0755, root, root) /usr/bin/ldoc
%attr (0755, root, root) /usr/bin/doc
%attr (0644, root, root) /usr/include/block.h
%attr (0644, root, root) /usr/include/blowfish.h
%attr (0644, root, root) /usr/include/cdb.h
%attr (0644, root, root) /usr/include/cdbpp.h
%attr (0644, root, root) /usr/include/container.h
%attr (0644, root, root) /usr/include/crc32.h
%attr (0644, root, root) /usr/include/md32_common.h
%attr (0644, root, root) /usr/include/test.h
%attr (0644, root, root) /usr/include/dcdb_config.h
%attr (0644, root, root) /usr/include/index.h
%attr (0644, root, root) /usr/include/mgetopt.h
%attr (0644, root, root) /usr/include/token.h
%attr (0644, root, root) /usr/include/interface.h
%attr (0644, root, root) /usr/include/sort.h
%attr (0644, root, root) /usr/include/test.h
%attr (0644, root, root) /usr/include/token.h
%attr (0644, root, root) /usr/include/win32Util.h
%attr (0644, root, root) /usr/include/blowfish/blowfish.h
%attr (0644, root, root) /usr/include/md5/md5.h
%attr (0644, root, root) /usr/include/md5/md5_locl.h
%attr (0644, root, root) /usr/include/sha1/sha.h
%attr (0644, root, root) /usr/include/sha1/sha_locl.h
%attr (0644, root, root) /usr/include/ripemd/ripemd.h
%attr (0644, root, root) /usr/include/ripemd/rmd_locl.h
%attr (0644, root, root) /usr/include/ripemd/rmdconst.h
%attr (0644, root, root) /usr/lib/libcdb-%{version}-%{release}.a
%attr (0755, root, root) /usr/lib/libcdb-%{version}-%{release}.so*

%files doc
#%attr (0644, root, root) /usr/doc/cdb.dvi
#%attr (0644, root, root) /usr/doc/sort.dvi
%doc COPYING ChangeLog INSTALL NEWS README
%doc doc/cdb.dvi doc/sort.dvi doc/cdb.tex doc/sort.tex


%files lib
%attr (0644, root, root) /usr/lib/libcdb-%{version}-%{release}.a
%attr (0755, root, root) /usr/lib/libcdb-%{version}-%{release}.so*

%files extra
%attr (0755, root, root) /usr/lib/libcdbtcl-%{version}-%{release}.so

