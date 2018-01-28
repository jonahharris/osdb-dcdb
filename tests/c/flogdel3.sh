#

cnt=1
keys=10000
delnum=2000
kincr=1000
dincr=250
endhere=100
if [ "$1" != "" ]; then
	endhere="$1"
fi

while [ 1  = 1 ]; do
	echo "Running #$cnt ..."
	echo "$keys keys, $delnum deleted each pass" 
	./mkdata $keys
	echo ""
	echo "--------------------------------------------------"
	echo "./flogdel3 $delnum"
	./flogdel3 $delnum
	if [ $? != 0 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flogdel3"
		exit 0
	fi
	keys=`expr $keys + $kincr`
	delnum=`expr $delnum + $dincr`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
