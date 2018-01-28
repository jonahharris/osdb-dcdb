#

LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
while [ $cnt -le $COUNT ]; do
	echo "50000 keys for #$cnt ..."
	rm -f flogdel3.db
	rm -f fd3_ssnidx.i?x
	rm -f fd3_lnidx.i?x
	rm -f fd3_lfname.i?x
	rm -f testAdd.txt
	./mkdata
	echo "running #$cnt ..."
	./flogdel3
	if [ $? != 0 ]; then
		echo " failed!"
		exit 1
	fi
	echo ""
	echo "Passed!"

	cnt=`expr $cnt + 1`
done
