#

cnt=1
keys=10
incr=1
endhere=20
if [ "$1" != "" ]; then
  endhere="$1"
fi

while [ 1  = 1 ]; do
  echo "Running #$cnt"
  echo "$keys iterations"
  echo ""
  echo "--------------------------------------------------"
  ./flognum1.py $keys
  if [ $? == 1 ]; then
    echo " failed!"
    exit 1
  fi
  if [ $cnt == $endhere ]; then
    echo "Finished flogcont1."
    exit 0
  fi
  keys=`expr $keys + $incr`
  echo "--------------------------------------------------"
  echo ""
  echo "Passed"
  echo ""

  cnt=`expr $cnt + 1`
done
