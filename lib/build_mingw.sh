#!/bin/sh
CC=/c/MinGW/bin/gcc.exe
CFLAGS="-O2 -s -march=pentium4 -shared -I../include -finline-functions -funroll-loops -fno-strength-reduce -D__cygwin__"
DCDB_OBJECTS="bcnum.o block.o cdbadd.o cdbdf.o cdbedit.o cdbindex.o cdbmove.o cdbpack.o cdbtable.o cdbui.o cdbutils.o idxblk.o index.o number.o qsort.o sort.o token.o cdbws.o crc32.o"

echo "$CC $CFLAGS -c bcnum.c"
$CC $CFLAGS -c bcnum.c
echo "$CC $CFLAGS -c block.c"
$CC $CFLAGS -c block.c
echo "$CC $CFLAGS -c cdbadd.c"
$CC $CFLAGS -c cdbadd.c
echo "$CC $CFLAGS -c cdbdf.c"
$CC $CFLAGS -c cdbdf.c
echo "$CC $CFLAGS -c cdbedit.c"
$CC $CFLAGS -c cdbedit.c
echo "$CC $CFLAGS -c cdbindex.c"
$CC $CFLAGS -c cdbindex.c
echo "$CC $CFLAGS -c cdbmove.c"
$CC $CFLAGS -c cdbmove.c
echo "$CC $CFLAGS -c cdbpack.c"
$CC $CFLAGS -c cdbpack.c
echo "$CC $CFLAGS -c cdbtable.c"
$CC $CFLAGS -c cdbtable.c
echo "$CC $CFLAGS -c cdbui.c"
$CC $CFLAGS -c cdbui.c
echo "$CC $CFLAGS -c cdbutils.c"
$CC $CFLAGS -c cdbutils.c
echo "$CC $CFLAGS -c idxblk.c"
$CC $CFLAGS -c idxblk.c
echo "$CC $CFLAGS -c index.c"
$CC $CFLAGS -c index.c
echo "$CC $CFLAGS -c number.c"
$CC $CFLAGS -c number.c
echo "$CC $CFLAGS -c qsort.c"
$CC $CFLAGS -c qsort.c
echo "$CC $CFLAGS -c sort.c"
$CC $CFLAGS -c sort.c
echo "$CC $CFLAGS -c token.c"
$CC $CFLAGS -c token.c
echo "$CC $CFLAGS -c cdbws.c"
$CC $CFLAGS -c cdbws.c
echo "$CC $CFLAGS -c crc32.c"
$CC $CFLAGS -c crc32.c

swig -tcl cdbtcl_mgw.i
echo "$CC $CFLAGS -o cdbtcl.dll -I/c/Tcl/include cdbtcl_mgw_wrap.c cdb.c container.c $DCDB_OBJECTS /c/tcl/bin/tcl84.dll"
$CC $CFLAGS -o cdbtcl.dll -I/c/Tcl/include -I../include cdbtcl_mgw_wrap.c cdb.c container.c $DCDB_OBJECTS /c/tcl/bin/tcl84.dll

