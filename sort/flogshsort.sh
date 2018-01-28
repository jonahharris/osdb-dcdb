#

cnt=1
keys=500000
lengths=24
while [ 1  = 1 ]; do
	echo "$keys keys for #$cnt ..."
	./words $keys $lengths > inputshl
	echo "sorting..."
	sort inputshl > inputshl.srt
	echo "running #$cnt ..."
	./flogshsort inputshl $keys > inputshl.out
	if [ $? != 0 ]; then
		echo " failed!"
		echo "inputshl producing the failure is left in \"inputshl\""
		exit 1
	fi
	echo "difference (should be nothing)..."
	diff inputshl.out inputshl.srt
	echo "Passed!"

	cnt=`expr $cnt + 1`
	sync
	sync
done
