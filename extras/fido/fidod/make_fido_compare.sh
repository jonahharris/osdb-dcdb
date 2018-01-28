gcc -I../../../include -Wall -O3 -s -march=i686 -mcpu=i686 -Wall -fno-strength-reduce -o fido_compare fido_compare.c ../../../lib/cdb.c ../../../lib/.libs/libcdb.a -lcrypto -lssl
