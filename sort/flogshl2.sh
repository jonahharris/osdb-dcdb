#

cnt=1
keys=100000
lengths=10
while [ 1  = 1 ]; do
	echo "$keys keys for #$cnt ..."
	./words $keys $lengths > inputshl
	echo "making dups..."
	./mkdups inputshl inputshl.1 5
	./mkdups inputshl.1 inputshl 4
	./mkdups inputshl inputshl.1 3
	./mkdups inputshl.1 inputshl 2
	rm inputshl.1
	echo "sorting..."
	sort inputshl > inputshl.srt
	echo "reverse sorting..."
	sort -r inputshl > inputshl.rev.srt
	echo "running #$cnt ..."
	./flogshl2 inputshl inputshl.srt inputshl.rev.srt
	if [ $? != 0 ]; then
		echo " failed!"
		echo "inputshl producing the failure is left in \"inputshl\""
		exit 1
	fi
	echo "Passed!"

	cnt=`expr $cnt + 1`
	sync
	sync
done
