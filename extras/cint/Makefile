
### Change these.
CINTSYSDIR=/opt/cint5.16/lib
CINTBIN = /opt/cint5.16/bin/cint
CINTMAKEBIN = /opt/cint5.16/bin/cintmake
###

all:	mycint

INCLUDE = ../../include
SOURCES = ../../lib/test.c ../../lib/qsort.c ../../lib/token.c \
    ../../lib/block.c ../../lib/idxblk.c ../../lib/index.c ../../lib/cdbadd.c \
    ../../lib/cdbindex.c ../../lib/cdbedit.c ../../lib/cdbmove.c \
    ../../lib/cdbtable.c ../../lib/cdbutils.c ../../lib/cdbpack.c \
    ../../lib/cdbui.c ../../lib/cdbws.c ../../lib/cdbdf.c ../../lib/md5.c \
    ../../lib/md5_dgst.c ../../lib/md5_one.c ../../lib/sha1.c \
    ../../lib/sha1_one.c ../../lib/sha1dgst.c ../../lib/rmd160.c \
    ../../lib/rmd_dgst.c ../../lib/rmd_one.c ../../lib/blowfish.c \
    ../../lib/crc32.c  ../../lib/sort.c ../../lib/cdb.c \
    ../../lib/container.c ../../lib/bcnum.c ../../lib/number.c

Makefile.cint:	cint_cdb.h ../../lib/cdb.c ../../lib/container.c
	CINTSYSDIR=$(CINTSYSDIR) CINTBIN=$(CINTBIN) makecint -mk Makefile.cint -I../../include -o mycint -c cint_cdb.h
	-sh mk_mycint.sh

mycint:	Makefile.cint $(SOURCES)
	CINTSYSDIR=$(CINTSYSDIR) CINTBIN=$(CINTBIN) make -f Makefile.cint

clean:
	-rm -f cdb.o container.o *.db *.i?x G__c_mycint.c G__c_mycint.o Makefile.cint G__c_mycint.h G__setup.o mycint
