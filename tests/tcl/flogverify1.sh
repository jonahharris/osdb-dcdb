#

cnt=1
endhere=50
if [ "$1" != "" ]; then
	endhere="$1"
fi
if [ "$2" != "" ]; then
	#set up some initial files first.
	init_cnt=0
	thisdir=`pwd`
	cd ../../lib
	echo "Generating initial files..."
	while [ $init_cnt -lt $2 ]; do
		ls *.{c,i} | xargs -i cp {} ../tests/tcl/bogus_data/$init_cnt.{}
		init_cnt=`expr $init_cnt + 1`
		stat=`expr $init_cnt % 10`
		if [ $stat = 0 ]; then
			echo -n "."
		fi
	done
	cd $thisdir
	echo ""
	echo "Done generating initial files."
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
	echo "set numIteration $cnt" > flogverify1_inc.tcl
	echo ""
	echo "--------------------------------------------------"
	$TCLSH flogverify1.tcl
	if [ $? == 1 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flogverify1."
		echo ""
		echo "Cleaning up temporary files ..."
		rm -f flogverify1_inc.tcl
		rm -f flogverify1_list.txt
		echo "Cleaning up bogus_data/ ...."
		find bogus_data/ -type f | xargs -n200 rm -f
		echo ""
		echo "Done."
		exit 0
	fi
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done

