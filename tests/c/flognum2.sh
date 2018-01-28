#

cnt=1
keys=100
incr=10
endhere=100
if [ "$1" != "" ]; then
	endhere="$1"
fi

#uname_s=`uname -s`
#if [ "$uname_s" == "SunOS" ]; then
#  exit 0
#fi

while [ 1  = 1 ]; do
	echo "Running #$cnt ..."
	echo ""
	echo "--------------------------------------------------"
	echo "Running with $keys operations..."
	./flognum2-gen $keys
	if [ $? != 0 ]; then
	  echo "./flognum2-gen $keys failed"
		exit 1
	fi
	for ln in `cat testAdd.txt`; do
	  scale=`echo $ln | cut -f1 -d':'`
		num1=`echo $ln | cut -f2 -d':'`
		op=`echo $ln | cut -f3 -d':'`
		num2=`echo $ln | cut -f4 -d':'`
		#echo "./flognum2 $scale $num1 $op $num2"
		echo -n "."
	  val=`./flognum2 $scale $num1 $op $num2`
	  if [ $? != 0 ]; then
  		echo " \"./flognum2 $scale $num1 $op $num2\" failed!"
  		exit 1
  	fi
		# Now, run bc with these data sets and compare with val.
		echo "scale = $scale" >flognum2.bc
		echo "total = 0" >> flognum2.bc
		if [ $op == "x" ]; then
			op="*"
		fi
		echo "total = $num1 $op $num2" >> flognum2.bc
		echo "total /= 1" >> flognum2.bc
		echo "if (total != $val) {" >> flognum2.bc
		echo "  print \"failed\n\"" >> flognum2.bc
		echo "}" >> flognum2.bc
		echo "if (total == $val) {" >> flognum2.bc
		echo "print \"passed\n\"" >> flognum2.bc
		echo "}" >> flognum2.bc
		echo "quit" >> flognum2.bc
		val2=`bc -q flognum2.bc`
		if [ "$val2" != "passed" ]; then
		  echo "./flognum2 $scale $num1 $op $num2 did not match bc's output"
		fi
	done
	echo ""
	rm -f flognum2.bc
	if [ $cnt == $endhere ]; then
		echo "Finished flognum2"
		exit 0
	fi
	keys=`expr $keys + $incr`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
