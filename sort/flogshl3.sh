#

cnt=1
keys=250000
lengths=10
while [ 1  = 1 ]; do
	echo "$keys keys for #$cnt ..."
	./words $keys $lengths > inputshl
	echo "sorting..."
	sort -f inputshl > inputshl.srt
	echo "reverse sorting..."
	sort -fr inputshl > inputshl.rev.srt
	echo "running #$cnt ..."
#	cint -p ./flogshl3.c $keys inputshl inputshl.srt inputshl.rev.srt
	./flogshl3 $keys inputshl inputshl.srt inputshl.rev.srt
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
