#

cnt=1
keys=10000
kincr=1000
endhere=100
if [ "$1" != "" ]; then
	endhere="$1"
fi

while [ 1  = 1 ]; do
	echo "Running #$cnt ..."
	echo "$keys keys"
	./mkdata.py $keys
	mv testAdd.txt testAdd2.txt
	./mkdata.py $keys
	echo ""
	echo "--------------------------------------------------"
	./flogupd1.py
	if [ $? == 1 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flogupd1."
		exit 0
	fi
	keys=`expr $keys + $kincr`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
