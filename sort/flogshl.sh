#

cnt=1
keys=50000
lengths=35
while [ 1  = 1 ]; do
	echo "$keys keys for #$cnt ..."
	./words $keys $lengths > inputshl
	# Uncomment the following if you want to test dups.
	echo "making dups..."
	./mkdups inputshl inputshl.1 37
	./mkdups inputshl.1 inputshl 36
	./mkdups inputshl inputshl.1 35
	./mkdups inputshl.1 inputshl 34
	./mkdups inputshl inputshl.1 33
	./mkdups inputshl.1 inputshl 32
	./mkdups inputshl inputshl.1 31
	./mkdups inputshl.1 inputshl 30
	./mkdups inputshl inputshl.1 29
	./mkdups inputshl.1 inputshl 28
	./mkdups inputshl inputshl.1 27
	./mkdups inputshl.1 inputshl 26
	./mkdups inputshl inputshl.1 25
	./mkdups inputshl.1 inputshl 24
	./mkdups inputshl inputshl.1 23
	./mkdups inputshl.1 inputshl 22
	./mkdups inputshl inputshl.1 21
	./mkdups inputshl.1 inputshl 20
	./mkdups inputshl inputshl.1 19
	./mkdups inputshl.1 inputshl 18
	./mkdups inputshl inputshl.1 17
	./mkdups inputshl.1 inputshl 16
	./mkdups inputshl inputshl.1 15
	./mkdups inputshl.1 inputshl 14
	./mkdups inputshl inputshl.1 13
	./mkdups inputshl.1 inputshl 12
	./mkdups inputshl inputshl.1 11
	./mkdups inputshl.1 inputshl 10
	./mkdups inputshl inputshl.1 9
	./mkdups inputshl.1 inputshl 8
	./mkdups inputshl inputshl.1 7
	./mkdups inputshl.1 inputshl 6
	./mkdups inputshl inputshl.1 5
	./mkdups inputshl.1 inputshl 4
	./mkdups inputshl inputshl.1 3
	./mkdups inputshl.1 inputshl 2
	rm -f inputshl.1
	echo "sorting..."
	sort inputshl > inputshl.srt
	echo "reverse sorting..."
	sort -r inputshl > inputshl.rev.srt
	echo "running #$cnt ..."
	./flogshl inputshl inputshl.srt inputshl.rev.srt
	if [ $? != 0 ]; then
		echo " failed!"
		echo "inputshl producing the failure is left in \"inputshl\""
		exit 1
	fi
	sleep 5
	echo "Passed!"

	cnt=`expr $cnt + 1`
	sync
	sync
done
