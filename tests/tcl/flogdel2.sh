#

cnt=1
keys=10000
delnum=1000
kincr=1000
dincr=1000
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
	echo "Running #$cnt ..."
	echo "$keys keys, $delnum deleted each pass" 
	echo "set delNumber $delnum" > flogdel2_inc.tcl
	$TCLSH ./mkdata.tcl $keys
	echo ""
	echo "--------------------------------------------------"
	$TCLSH flogdel2.tcl
	if [ $? == 1 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flogdel2"
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
