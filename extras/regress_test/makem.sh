#!/bin/bash

if [ "x$CFLAGS" == "x" ]; then
  echo "Set CFLAGS before you run this."
  echo "For example: "
  echo ""
  echo "export CFLAGS=\"-O2 -s -march=i686 -mcpu=i686\""
  echo "bash makem.sh"
  echo ""
  echo "That should do it."
fi

if [ ! -f ../../lib/libcdb.so ]; then
  echo "../../libcdb.so doesn't exist"
  echo "Make it first and then try this again."
  exit 1
fi

echo "gcc $CFLAGS -o mkdups mkdups.c"
gcc $CFLAGS -o mkdups mkdups.c
echo "gcc $CFLAGS -o words words.c"
gcc $CFLAGS -o words words.c
echo "gcc $CFLAGS -I../../include -o mkdata mkdata.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o mkdata mkdata.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o mk2data mk2data.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o mk2data mk2data.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o mk3data mk3data.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o mk3data mk3data.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o mksdata mksdata.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o mksdata mksdata.c -L../../lib ../../lib/libcdb.a

echo "gcc $CFLAGS -I../../include -o flogadd flogadd.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogadd flogadd.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogadd2 flogadd2.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogadd2 flogadd2.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogdel flogdel.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogdel flogdel.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogdel2 flogdel2.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogdel2 flogdel2.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogdel3 flogdel3.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogdel3 flogdel3.c -L../../lib ../../lib/libcdb.a

echo "gcc $CFLAGS -I../../include -o flogdelidx flogdelidx.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogdelidx flogdelidx.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogdelidx2 flogdelidx2.c -L../../lib ../../lib/libcdb.a"

gcc $CFLAGS -I../../include -o flogdelidx2 flogdelidx2.c -L../../lib ../../lib/libcdb.a

echo "gcc $CFLAGS -I../../include -o flogidx flogidx.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogidx flogidx.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o floglhshl floglhshl.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o floglhshl floglhshl.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o floglhshl2 floglhshl2.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o floglhshl2 floglhshl2.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogndelidx flogndelidx.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogndelidx flogndelidx.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flognewadd flognewadd.c -L../../lib ../../libcdb.a"
gcc $CFLAGS -I../../include -o flognewadd flognewadd.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flognidx flognidx.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flognidx flognidx.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flognshl flognshl.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flognshl flognshl.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogridx flogridx.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogridx flogridx.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogrmshl flogrmshl.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogrmshl flogrmshl.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogshl flogshl.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogshl flogshl.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogshl2 flogshl2.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogshl2 flogshl2.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogshl3 flogshl3.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogshl3 flogshl3.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogshlq flogshlq.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogshlq flogshlq.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogshlq2 flogshlq2.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogshlq2 flogshlq2.c -L../../lib ../../lib/libcdb.a
echo "gcc $CFLAGS -I../../include -o flogshsort flogshsort.c -L../../lib ../../lib/libcdb.a"
gcc $CFLAGS -I../../include -o flogshsort flogshsort.c -L../../lib ../../lib/libcdb.a
