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
	tcc -I../../include -L../../lib "-run -lcdbtcl" mkdata.c $keys
	echo ""
	echo "--------------------------------------------------"
	echo "./flogdel2 $delnum"
	tcc -I../../include -L../../lib "-run -lcdbtcl" flogdel2.c $delnum
	if [ $? != 0 ]; then
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
