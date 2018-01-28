#!/bin/sh
# For Linux
CFLAGS="-I../../include -Wall -O3 -s -march=pentium4 -mcpu=pentium4 -fno-strength-reduce  -D__linux__"
# Solaris
#CFLAGS="-I../../include -Wall -O3 -s -msupersparc -D__solaris__"
# Cygwin
#CFLAGS="-I../../include -Wall -O3 -s -march=i686 -mcpu=i686 -D__cygwin__"
mv Makefile.cint tmp_mkfile
cat tmp_mkfile | sed 's/LIBS       =/LIBS       = test.o qsort.o token.o block.o idxblk.o index.o cdbadd.o cdbindex.o cdbedit.o cdbmove.o cdbtable.o cdbutils.o cdbpack.o cdbui.o cdbws.o cdbdf.o md5.o md5_dgst.o md5_one.o sha1.o sha1_one.o sha1dgst.o rmd160.o rmd_dgst.o rmd_one.o blowfish.o crc32.o  sort.o cdb.o container.o bcnum.o number.o -Llcrypto -lssl -lpthread/g' > Makefile.cint
for fil in ../../lib/test.c ../../lib/qsort.c ../../lib/token.c ../../lib/block.c \
      ../../lib/idxblk.c ../../lib/index.c ../../lib/cdbadd.c ../../lib/cdbindex.c \
      ../../lib/cdbedit.c ../../lib/cdbmove.c ../../lib/cdbtable.c ../../lib/cdbutils.c \
      ../../lib/cdbpack.c ../../lib/cdbui.c ../../lib/cdbws.c ../../lib/cdbdf.c \
      ../../lib/md5.c ../../lib/md5_dgst.c ../../lib/md5_one.c ../../lib/sha1.c \
      ../../lib/sha1_one.c ../../lib/sha1dgst.c ../../lib/rmd160.c \
      ../../lib/rmd_dgst.c ../../lib/rmd_one.c ../../lib/blowfish.c \
      ../../lib/crc32.c  ../../lib/sort.c ../../lib/cdb.c \
      ../../lib/container.c ../../lib/bcnum.c ../../lib/number.c
do
  echo "gcc $CFLAGS -c $fil"
  gcc $CFLAGS -c $fil
done
rm -f tmp_mkfile
mv Makefile.cint tmp_mkfile
cat tmp_mkfile | sed 's/$(CINTSYSDIR)\/cint/$(CINTBIN)/g' > Makefile.cint
rm -f tmp_mkfile
