#

cnt=1
keys=20000
lengths=10
while [ 1  = 1 ]; do
	echo "$keys keys for #$cnt ..."
	./words $keys $lengths > input
	echo "" >>input
	echo "Making dups"
	#
	# This seems like overkill, but it will exercise the duplicate
	# handling capability of the algorith.
	#
	./mkdups input input.dup.1 59
	./mkdups input.dup.1 input.dup 58
	./mkdups input.dup input.dup.1 57
	./mkdups input.dup.1 input.dup 56
	./mkdups input.dup input.dup.1 55
	./mkdups input.dup.1 input.dup 54
	./mkdups input.dup input.dup.1 53
	./mkdups input.dup.1 input.dup 52
	./mkdups input.dup input.dup.1 51
	./mkdups input.dup.1 input.dup 50
	./mkdups input.dup input.dup.1 49
	./mkdups input.dup.1 input.dup 48
	./mkdups input.dup input.dup.1 47
	./mkdups input.dup.1 input.dup 46
	./mkdups input.dup input.dup.1 45
	./mkdups input.dup.1 input.dup 44
	./mkdups input.dup input.dup.1 43
	./mkdups input.dup.1 input.dup 42
	./mkdups input.dup input.dup.1 41
	./mkdups input.dup.1 input.dup 40
	./mkdups input.dup input.dup.1 39
	./mkdups input.dup.1 input.dup 38
	./mkdups input.dup input.dup.1 37
	./mkdups input.dup.1 input.dup 36
	./mkdups input.dup input.dup.1 35
	./mkdups input.dup.1 input.dup 34
	./mkdups input.dup input.dup.1 33
	./mkdups input.dup.1 input.dup 32
	./mkdups input.dup input.dup.1 31
	./mkdups input.dup.1 input.dup 30
	./mkdups input.dup input.dup.1 6
	./mkdups input.dup.1 input.dup 5
	./mkdups input.dup input.dup.1 4
	./mkdups input.dup.1 input.dup 3
	echo "Sorting..."
	rm input.dup.1
	sort input.dup > input.srt
	sort -r input.dup > input.rev.srt
	echo "running #$cnt ..."
	./flogndelidx2 input.dup
	if [ $? != 0 ]; then
		echo " failed!"
		echo "input producing the failure is left in \"input.dup\""
		exit 1
	fi
	echo "Passed!"

	cnt=`expr $cnt + 1`
	sync
	sync
done
