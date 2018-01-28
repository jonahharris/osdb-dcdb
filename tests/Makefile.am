bin_SCRIPTS = test_c.sh test_tcl.sh test_py.sh test_tcc.sh
# Add test_c.sh and test_pl.sh when the tests are completed.

SHELL=@SHELL@

test_c_DEPS = c/mkdata c/flogadd1 c/flogadd2 c/flogdel1 c/flogdel2 c/flogdel3 \
	      c/flogenc1 c/flogreindex1 c/flograwadd1 c/flogupd1 c/flognum1 \
	      c/floghash1 c/flogthrd1 c/words c/rough_sort \
				c/flognum2 c/flognum2-gen c/flogthrd2

c/mkdata:	c/mkdata.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/mkdata c/mkdata.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogadd1:	c/flogadd1.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogadd1 c/flogadd1.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flograwadd1:	c/flograwadd1.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flograwadd1 c/flograwadd1.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogadd2:	c/flogadd2.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogadd2 c/flogadd2.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogdel1:	c/flogdel1.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogdel1 c/flogdel1.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogdel2:	c/flogdel2.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogdel2 c/flogdel2.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogdel3:	c/flogdel3.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogdel3 c/flogdel3.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogenc1:	c/flogenc1.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogenc1 c/flogenc1.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogreindex1:	c/flogreindex1.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogreindex1 c/flogreindex1.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogupd1:	c/flogupd1.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flogupd1 c/flogupd1.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flognum1:	c/flognum1.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flognum1 c/flognum1.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/floghash1:	c/floghash1.c
	$(CC) $(CFLAGS) -I../include -o c/floghash1 c/floghash1.c ../lib/libcdb.a -lpthread

c/flogthrd1:	c/flogthrd1.c
	$(CC) $(CFLAGS) -I../include -o c/flogthrd1 c/flogthrd1.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/words:	c/words.c
	$(CC) $(CFLAGS) -I../include -o c/words c/words.c ../lib/libcdb.a -lpthread

c/rough_sort:	c/rough_sort.c
	$(CC) $(CFLAGS) -I../include -o c/rough_sort c/rough_sort.c ../lib/libcdb.a -lpthread

c/flognum2:	c/flognum2.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flognum2 c/flognum2.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flognum2-gen:	c/flognum2-gen.c ../lib/cdb.c
	$(CC) $(CFLAGS) -I../include -o c/flognum2-gen c/flognum2-gen.c ../lib/cdb.c ../lib/libcdb.a -lssl -lcrypto -lpthread

c/flogthrd2:	c/flogthrd2.c
	$(CC) $(CFLAGS) -I../include -o c/flogthrd2 c/flogthrd2.c ../lib/libcdb.a -lssl -lcrypto -lpthread

test_c.sh: test_c.sh.in $(test_c_DEPS)
	-rm -f test_c.sh
	-cp test_c.sh.in test_c.sh
	-$(SHELL) test_c.sh 10
	-touch test_c.sh.in

test_tcl.sh: test_tcl.sh.in
	-rm -f test_tcl.sh
	-cp test_tcl.sh.in test_tcl.sh
	-$(SHELL) test_tcl.sh 10
	-touch test_tcl.sh.in

test_py.sh: test_py.sh.in
	-rm -f test_py.sh
	-cp test_py.sh.in test_py.sh
	-$(SHELL) test_py.sh 10
	-touch test_py.sh.in

test_tcc.sh: test_tcc.sh.in
	-rm -f test_tcc.sh
	-cp test_tcc.sh.in test_tcc.sh
	-$(SHELL) test_tcc.sh 10
	-touch test_tcc.sh.in

clean:		cleandb
cleandb:
	-rm -f c/*.db c/*.i?x c/*.LCK c/*.txt c/*.output c/*.df \
		c/encrypted.txt \
		c/mkdata c/flogadd1 c/flogadd2 c/flogdel1 \
		c/flogdel2 c/flogdel3 c/flogenc1 c/flogreindex1 \
		c/flogupd1 c/flograwadd1 c/flognum1 c/*.exe \
		c/floghash1 c/input* c/words c/rough_sort \
		c/flogthrd1 c/flognum2 c/flognum2-gen \
		c/flogthrd2 \
		tcl/*.db tcl/*.i?x tcl/*.LCK tcl/*.txt tcl/*.df \
		tcl/encrypted.txt \
		python/*.db python/*.i?x python/*.LCK python/*.txt \
		python/*.df python/encrypted.txt \
		tcc/*.db tcc/*.i?x tcc/*.LCK tcc/*.txt tcc/*.df \
		tcc/*.txt tcc/*.output tcc/*.df tcc/input* \
		test_c.sh test_tcl.sh test_tcc.sh
	-find tcl/bogus_data -type f | xargs -n200 rm -f
