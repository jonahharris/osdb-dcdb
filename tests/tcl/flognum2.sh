#

cnt=1
keys=1000
incr=10
endhere=100
if [ "$1" != "" ]; then
	endhere="$1"
fi

uname_s=`uname -s`
OS="Unknown"
if [ "$uname_s" == "Linux" ]; then
  OS="Linux"
fi
if [ "$uname_s" == "SunOS" ]; then
  OS="Solaris"
fi
if [ "$OS" == "Unknown" ]; then
  uname_o=`uname -o`
  if [ "$uname_o" == "Cygwin" ]; then
    OS="Cygwin"
  fi
fi

if [ "$OS" == "Linux" ]; then
  TCLSH=tclsh8.4
fi
if [ "$OS" == "Solaris" ]; then
  TCLSH=tclsh8.4
fi
if [ "$OS" == "Cygwin" ]; then
  TCLSH=tclsh
fi

while [ 1  = 1 ]; do
	echo "Running #$cnt ..."
	echo ""
	echo "--------------------------------------------------"
	echo "Running with $keys operations..."
	$TCLSH flognum2-gen.tcl $keys
	if [ $? != 0 ]; then
	  echo "$TCLSH flognum2-gen.tcl $keys failed"
		exit 1
	fi
	for ln in `cat testAdd.txt`; do
	  scale=`echo $ln | cut -f1 -d':'`
		num1=`echo $ln | cut -f2 -d':'`
		op=`echo $ln | cut -f3 -d':'`
		num2=`echo $ln | cut -f4 -d':'`
		echo -n "."
	  val=`$TCLSH flognum2.tcl $scale $num1 $op $num2`
		result=$?
	  if [ $result -lt 0 ]; then
  		echo " \"$TCLSH flognum2.tcl $scale $num1 $op $num2\" failed!"
  		exit 1
  	fi
		if [ $result -eq 4 ]; then
		  continue
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
		  echo "$TCLSH flognum2.tcl $scale $num1 $op $num2 did not match bc's output"
			exit 1
		fi
	done
	echo ""
	rm -f flognum2.bc
	if [ $cnt == $endhere ]; then
		echo "Finished flognum1"
		exit 0
	fi
	keys=`expr $keys + $incr`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
