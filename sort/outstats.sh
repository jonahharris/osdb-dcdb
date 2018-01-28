#

cnt=1
keys=250000
lengths=24
iterations=100
rm -f outstats.out
while [ $cnt -le $iterations ]; do
	echo "$keys keys for #$cnt ..."
	./words $keys $lengths > inputshl
	echo "running #$cnt ..."
	./outstats inputshl $keys >> outstats.out
	if [ $? != 0 ]; then
		echo " failed!"
		echo "inputshl producing the failure is left in \"inputshl\""
		exit 1
	fi
	echo "Passed!"

	cnt=`expr $cnt + 1`
done
