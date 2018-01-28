#

cnt=1
keys=10000
incr=1000
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
	TCLSH=tclsh
fi
if [ "$OS" == "Solaris" ]; then
	TCLSH=tclsh8.4
fi
if [ "$OS" == "Cygwin" ]; then
	TCLSH=tclsh
fi

while [ 1  = 1 ]; do
	echo "Running #$cnt"
	echo "$keys keys"
	echo "set numLoop $keys" > flogadd2_inc.tcl
	echo ""
	echo "--------------------------------------------------"
	$TCLSH flogadd2.tcl
	if [ $? == 1 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flogadd2."
		exit 0
	fi
	keys=`expr $keys + $incr`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
