#

cnt=1
keys=5
endhere=100
if [ "$1" != "" ]; then
	endhere="$1"
fi

uname_s=`uname -s`
if [ "$uname_s" == "SunOS" ]; then
  exit 0
fi

while [ 1  = 1 ]; do
	echo "Running #$cnt ..."
	echo "$keys increments for each pass" 
	echo ""
	echo "--------------------------------------------------"
	echo "./flognum1 $keys"
	./flognum1 $keys
	if [ $? != 0 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flognum1"
		exit 0
	fi
	keys=`expr $keys + 1`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
