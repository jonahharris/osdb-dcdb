#!/sbin/sh
if [ ! -x thrashvm ]; then
  if [ -f thrashvm.c ]; then
    echo "Building thrashvm from thrashvm.c"
    gcc -O2 -s -o thrashvm thrashvm.c
  fi
fi

if [ ! -x thrashvm ]; then
  echo "Couldn't create thrashvm from thrashvm.c"
  exit 1
fi

outctr=0
sleeptime=600
numthrashers=10
while [ $outctr -lt 50 ]; do
	val=4
	ctr=1
	echo ""
	echo "Starting $numthrashers instances of thrashvm."
	echo ""
	while [ $ctr -le $numthrashers ]; do
		echo "Starting thrashvm #$ctr..."
		./thrashvm &
		sleep $val
		ctr=`expr $ctr + 1`
		val=`expr $val + 1`
	done
	outctr=`expr $outctr + 1`

	echo ""
	echo "Sleeping for $sleeptime seconds..."
	sleep $sleeptime
done
