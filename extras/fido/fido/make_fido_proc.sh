# You might need this version of you have more than, say, 50,000 excludes.
echo "gcc -DHASH_SEARCH -I../../../include $CFLAGS -o fido_proc fido_proc.c ../../../lib//libcdb.a"
gcc -DHASH_SEARCH -I../../../include $CFLAGS -o fido_proc fido_proc.c ../../../lib//libcdb.a
# Now, make rough_sort
echo "gcc I../../../include $CFLAGS -o rough_sort rough_sort.c ../../../lib//libcdb.a"
gcc -DHASH_SEARCH -I../../../include $CFLAGS -o rough_sort rough_sort.c ../../../lib//libcdb.a
