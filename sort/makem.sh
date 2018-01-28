echo "gcc $CFLAGS -o words words.c"
gcc $CFLAGS -o words words.c
echo "gcc $CFLAGS -o mkdups mkdups.c"
gcc $CFLAGS -o mkdups mkdups.c
echo "gcc $CFLAGS -I. -o flogshl flogshl.c sort.c qsort.c"
gcc $CFLAGS -I. -DVERBOSE -o flogshl flogshl.c sort.c qsort.c -lpthread
echo "gcc $CFLAGS -I. -o flogshl2 flogshl2.c sort.c qsort.c"
gcc $CFLAGS -I. -o flogshl2 flogshl2.c sort.c qsort.c -lpthread
echo "gcc $CFLAGS -I. -o flogshl3 flogshl3.c sort.c qsort.c"
gcc $CFLAGS -I. -o flogshl3 flogshl3.c sort.c qsort.c -lpthread
echo "gcc $CFLAGS -I. -o flogshsort flogshsort.c sort.c qsort.c"
gcc $CFLAGS -I. -o flogshsort flogshsort.c sort.c qsort.c -lpthread
echo "gcc $CFLAGS -I. -o outstats outstats.c sort.c qsort.c"
gcc $CFLAGS -I. -o outstats outstats.c sort.c qsort.c -lpthread
