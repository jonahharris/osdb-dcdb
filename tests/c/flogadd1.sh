#

cnt=1
keys=10000
incr=1000
endhere=100
if [ "$1" != "" ]; then
	endhere="$1"
fi

while [ 1  = 1 ]; do
	echo "Running #$cnt"
	echo "$keys keys"
	./mkdata $keys
	echo ""
	echo "--------------------------------------------------"
	./flogadd1
	if [ $? != 0 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flogadd1."
		exit 0
	fi
	keys=`expr $keys + $incr`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
