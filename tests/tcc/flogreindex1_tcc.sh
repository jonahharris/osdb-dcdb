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
	tcc -I../../include -L../../lib "-run -lcdbtcl" mkdata.c $keys
	echo ""
	echo "--------------------------------------------------"
	tcc -I../../include -L../../lib "-run -lcdbtcl" flogreindex1.c
	if [ $? != 0 ]; then
		echo " failed!"
		exit 1
	fi
	if [ $cnt == $endhere ]; then
		echo "Finished flogreindex1"
		exit 0
	fi
	keys=`expr $keys + $kincr`
	echo "--------------------------------------------------"
	echo ""
	echo "Passed"
	echo ""

	cnt=`expr $cnt + 1`
done
