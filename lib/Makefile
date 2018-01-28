# Makefile.in
#
SWIG = /usr/bin/swig
BUILD_DCDB_PERLMOD = yes
BUILD_DCDB_TCLMOD = yes
DCDB_PERL_INC = /usr/lib/perl/5.8/CORE
DCDB_PERL_LIBS = -lgdbm -lgdbm_compat -ldb -ldl -lm -lpthread -lc -lcrypt
DCDB_PYTHON_INC = /usr/include/python2.5
DCDB_PYTHON_LIB = /usr/lib
DCDB_PYTHON_LIBS = /usr/lib/libpython2.5.so
DCDB_TCL_MAJ = 8
DCDB_TCL_MIN = 4
DCDB_TCL_INC = /usr/include/tcl8.4
DCDB_TCL_LIB = /usr/lib
DCDB_TCL_LIBS = /usr/lib/libtcl8.4.so
DCDB_JNI_H = /opt/blackdown-jdk-1.4.2.01/include
DCDB_JNI_MD_H = /opt/blackdown-jdk-1.4.2.01/include/linux 
PV = 1.2-18
DCDB_CFLAGS = -O2 -s -march=pentium4 -mtune=pentium4 -finline-functions -fomit-frame-pointer -fno-strength-reduce -pipe -Wall -W -I${prefix}/include -D__linux__ -fPIC
LIBDCDB_LIBS = -lm -lssl -lcrypto -lpthread
CC = gcc
AR = ar
RANLIB = ranlib
DEST = /usr/local
INSTALL = /usr/bin/install -c
#shrext_cmds = .so
shrext = .so
top_srcdir = ..
includedir = @inscludedir@
mkinstalldirs = @mkinstalldirs@
tcl_maj = 8
tcl_min = 4

LIBCDBTCL = libcdbtcl-$(PV).$(shrext)
LIBCDB = libcdbtcl-$(PV).a
LIBCDB_SO = libcdb-$(PV).$(shrext)

LIBCDB_SOURCES = \
	test.c \
	qsort.c \
	token.c \
	block.c \
	idxblk.c \
	index.c \
	cdbadd.c \
	cdbindex.c \
	cdbedit.c \
	cdbmove.c \
	cdbtable.c \
	cdbutils.c \
	cdbpack.c \
	cdbui.c \
	cdbws.c \
	cdbdf.c \
	md5.c \
	md5_dgst.c \
	md5_one.c \
	sha1.c \
	sha1_one.c \
	sha1dgst.c \
	rmd160.c \
	rmd_dgst.c \
	rmd_one.c \
	blowfish.c \
	crc32.c \
	sort.c \
	bcnum.c \
	number.c

LIBDCDB_OBJECTS = \
	test.o \
	qsort.o \
	token.o \
	block.o \
	idxblk.o \
	index.o \
	cdbadd.o \
	cdbindex.o \
	cdbedit.o \
	cdbmove.o \
	cdbtable.o \
	cdbutils.o \
	cdbpack.o \
	cdbui.o \
	cdbws.o \
	cdbdf.o \
	md5.o \
	md5_dgst.o \
	md5_one.o \
	sha1.o \
	sha1_one.o \
	sha1dgst.o \
	rmd160.o \
	rmd_dgst.o \
	rmd_one.o \
	blowfish.o \
	crc32.o \
	sort.o \
	bcnum.o \
	number.o

HEADERS = \
	$(top_srcdir)/include/sort.h \
	$(top_srcdir)/include/block.h \
	$(top_srcdir)/include/cdb.h \
	$(top_srcdir)/include/cdbpp.h \
	$(top_srcdir)/include/container.h \
	$(top_srcdir)/include/crc32.h \
	$(top_srcdir)/include/index.h \
	$(top_srcdir)/include/interface.h \
	$(top_srcdir)/include/md32_common.h \
	$(top_srcdir)/include/mgetopt.h \
	$(top_srcdir)/include/test.h \
	$(top_srcdir)/include/mtoken.h \
	$(top_srcdir)/include/blowfish.h \
	$(top_srcdir)/include/dcdb_config.h \
	$(top_srcdir)/include/win32Util.h \
	$(top_srcdir)/include/md5/md5.h \
	$(top_srcdir)/include/md5/md5_locl.h \
	$(top_srcdir)/include/sha1/sha.h \
	$(top_srcdir)/include/sha1/sha_locl.h \
	$(top_srcdir)/include/ripemd/ripemd.h \
	$(top_srcdir)/include/ripemd/rmdconst.h \
	$(top_srcdir)/include/ripemd/rmd_locl.h \
	$(top_srcdir)/include/blowfish/blowfish.h \
	$(top_srcdir)/include/bcnum.h \
	$(top_srcdir)/include/number.h

USE_CFLAGS = $(DCDB_CFLAGS) -I$(top_srcdir)/include
LIBADD = $(LIBDCDB_LIBS)

.c.o:	$*.c
	$(CC) $(USE_CFLAGS) -c $<

all:	libcdb.a libcdb$(shrext)

install:	libcdb.a libcdb$(shrext) $(HEADERS)
	-mkdir -p $(DEST)/lib
	$(INSTALL) libcdb-$(PV).a $(DEST)/lib
	ln -s $(DEST)/lib/libcdb-$(PV).a $(DEST)/lib/libcdb.a
	$(INSTALL) libcdb-$(PV)$(shrext) $(DEST)/lib
	ln -s $(DEST)/lib/libcdb-$(PV)$(shrext) $(DEST)/lib/libcdb$(shrext)
	mkdir -p $(DEST)/include
	mkdir -p $(DEST)/include/md5
	mkdir -p $(DEST)/include/sha1
	mkdir -p $(DEST)/include/ripemd
	mkdir -p $(DEST)/include/blowfish
	$(INSTALL) -m 0644 $(top_srcdir)/include/sort.h $(DEST)/include
	$(INSTALL) -m 0644 $(top_srcdir)/include/block.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/cdb.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/cdbpp.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/container.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/crc32.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/index.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/interface.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/md32_common.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/mgetopt.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/test.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/mtoken.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/blowfish.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/dcdb_config.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/win32Util.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/md5/md5.h $(DEST)/include/md5
	$(INSTALL) -m 0644 $(top_srcdir)/include/md5/md5_locl.h $(DEST)/include/md5
	$(INSTALL) -m 0644 $(top_srcdir)/include/sha1/sha.h $(DEST)/include/sha1
	$(INSTALL) -m 0644 $(top_srcdir)/include/sha1/sha_locl.h $(DEST)/include/sha1
	$(INSTALL) -m 0644 $(top_srcdir)/include/ripemd/ripemd.h $(DEST)/include/ripemd
	$(INSTALL) -m 0644 $(top_srcdir)/include/ripemd/rmdconst.h $(DEST)/include/ripemd
	$(INSTALL) -m 0644 $(top_srcdir)/include/ripemd/rmd_locl.h $(DEST)/include/ripemd
	$(INSTALL) -m 0644 $(top_srcdir)/include/blowfish/blowfish.h $(DEST)/include/blowfish
	$(INSTALL) -m 0644 $(top_srcdir)/include/bcnum.h $(DEST)/include 
	$(INSTALL) -m 0644 $(top_srcdir)/include/number.h $(DEST)/include 

install_bindings:	libcdbtcl-$(PV)$(shrext) libconttcl-$(PV)$(shrext) libcdbpl-$(PV)$(shrext)
	$(INSTALL) libcdbtcl-$(PV)$(shrext) $(DEST)/lib
	-ln -s $(DEST)/lib/libcdbtcl-$(PV)$(shrext) $(DEST)/lib/libcdbtcl.so
	$(INSTALL) libconttcl-$(PV)$(shrext) $(DEST)/lib
	-ln -s $(DEST)/lib/libconttcl-$(PV)$(shrext) $(DEST)/lib/libconttcl.so
	$(INSTALL) libcdbpl-$(PV)$(shrext) $(DEST)/lib
	$(INSTALL) libcdbpl.pm $(DEST)/lib
	-ln -s $(DEST)/lib/libcdbpl-$(PV)$(shrext) $(DEST)/lib/libcdbpl.so

install_libcdbtcl:	libcdbtcl-$(PV)$(shrext)
	$(INSTALL) libcdbtcl-$(PV)$(shrext) $(DEST)/lib
	-ln -s $(DEST)/lib/libcdbtcl-$(PV)$(shrext) $(DEST)/lib/libcdbtcl.so

cdbtcl_wrap.c: cdbtcl.i
	-rm -f cdbtcl_wrap.c
	$(SWIG) -tcl cdbtcl.i

conttcl_wrap.c: conttcl.i
	-rm -f conttcl_wrap.c
	$(SWIG) -tcl conttcl.i

cdbpl_wrap.c:	cdbpl.i
	-rm -f cdbpl_wrap.c
	$(SWIG) -perl cdbpl.i

cdbpy_wrap.c:	cdbpy.i
	-rm -f cdbpy_wrap.c
	$(SWIG) -python cdbpy.i

cdbjava_wrap.c:	cdbjava.i
	-rm -f cdbjava_wrap.c
	$(SWIG) -java cdbjava.i

clean:
	-rm -f cdbtcl_wrap.c conttcl_wrap.c cdbpl_wrap.c cdbjava_wrap.c cdbpy_wrap.c \
	  *.a *.o *.so *.pm *.java *.class cdbtcl-$(PV) libcdb-$(PV)$(shrext) \
		libcdbtcl-$(PV)$(shrext) libconttcl-$(PV)$(shrext) \
		libcdbpl-$(PV)$(shrext) libcdbpy-$(PV)$(shrext)

distclean:	clean

libcdb.a:		libcdb-$(PV).a

libcdb-$(PV).a:		$(LIBDCDB_OBJECTS)
	$(AR) qc libcdb-$(PV).a $(LIBDCDB_OBJECTS)
	$(RANLIB) libcdb-$(PV).a
	-ln -s libcdb-$(PV).a libcdb.a

libcdb$(shrext):	libcdb-$(PV)$(shrext)

libcdb-$(PV)$(shrext):	$(LIBDCDB_OBJECTS)
	$(CC) $(USE_CFLAGS) -s -fPIC -shared -I$(top_srcdir)/include \
	-o libcdb-$(PV)$(shrext) $(LIBDCDB_OBJECTS) $(LIBADD)
	-ln -s libcdb-$(PV)$(shrext) libcdb$(shrext)

libcdbtcl$(shrext):	libcdbtcl-$(PV)$(shrext)

libcdbtcl-$(PV)$(shrext):	cdbtcl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS)
	$(CC) $(USE_CFLAGS) -s -fPIC -shared -I$(top_srcdir)/include -w \
	-I$(DCDB_TCL_INC) -o libcdbtcl-$(PV)$(shrext) \
	cdbtcl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS) \
	$(LIBADD) \
	$(DCDB_TCL_LIBS)
	-rm -f cdbtcl_wrap.c
	-ln -s libcdbtcl-$(PV)$(shrext) libcdbtcl$(shrext)

# Build a stand-alone executable for debugging.
cdbtcl:		cdbtcl-$(PV)

cdbtcl-$(PV):	cdbtcl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS)
	$(CC) $(USE_CFLAGS) -s -fPIC -I$(top_srcdir)/include -w \
	-I$(DCDB_TCL_INC) -o cdbtcl-$(PV) \
	cdbtcl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS) \
	$(LIBADD) \
	$(DCDB_TCL_LIBS)
	-rm -f cdbtcl_wrap.c

libconttcl$(shrext):	libconttcl-$(PV)$(shrext)

libconttcl-$(PV)$(shrext):	conttcl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS)
	$(CC) $(USE_CFLAGS) -s -fPIC -shared -I$(top_srcdir)/include -w \
	-I$(DCDB_TCL_INC) -o libconttcl-$(PV)$(shrext) \
	conttcl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS) \
	$(LIBADD) \
	$(DCDB_TCL_LIBS)
	-rm -f conttcl_wrap.c
	-ln -s libconttcl-$(PV)$(shrext) libconttcl$(shrext)

libcdbpl$(shrext):	libcdbpl-$(PV)$(shrext)

libcdbpl-$(PV)$(shrext):	cdbpl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS)
	$(CC) $(DCDB_CFLAGS) -s -Wall -fPIC -shared -I$(top_srcdir)/include -w \
	-I$(DCDB_PERL_INC) -o libcdbpl-$(PV)$(shrext) \
	cdbpl_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS) \
	$(LIBADD) \
	-L$(DCDB_PERL_INC) -lperl $(DCDB_PERL_LIBS)
	-rm -f cdbpl_wrap.c 
	-ln -s libcdbpl-$(PV)$(shrext) libcdbpl$(shrext)

libcdbpy$(shrext):	libcdbpy-$(PV)$(shrext)

libcdbpy-$(PV)$(shrext):	cdbpy_wrap.c cdb.c $(LIBDCDB_OBJECTS)
	$(CC) $(DCDB_CFLAGS) -s -Wall -fPIC -shared -I$(top_srcdir)/include -w \
	-I$(DCDB_PYTHON_INC) -o libcdbpy-$(PV)$(shrext) \
	cdbpy_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS) \
	$(LIBADD) \
	$(DCDB_PYTHON_LIBS)
	-rm -f cdbpy_wrap.c 
	-ln -s libcdbpy-$(PV)$(shrext) _cdbpy$(shrext)

libcdbjava$(shrext):	libcdbjava-$(PV)$(shrext)

libcdbjava-$(PV)$(shrext):	cdbjava_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS)
	$(CC) $(DCDB_CFLAGS) -s -Wall -fPIC -shared -I$(top_srcdir)/include -w \
	-I$(DCDB_JNI_H) \
	-I$(DCDB_JNI_MD_H) \
	-o libcdbjava-$(PV)$(shrext) \
	cdbjava_wrap.c cdb.c container.c $(LIBDCDB_OBJECTS) \
	$(LIBADD)
	-rm -f cdbpl_wrap.c 
	-ln -s libcdbjava-$(PV)$(shrext) libcdbjava$(shrext)

