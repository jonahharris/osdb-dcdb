#

cnt=1
keys=10000
delnum=1000
kincr=1000
dincr=250
endhere=100
if [ "$1" != "" ]; then
  endhere="$1"
fi

while [ 1  = 1 ]; do
  echo "Running #$cnt ..."
  echo "$keys keys, $delnum deleted each pass" 
  echo "delNumber = $delnum" > fa_inc.py
  ./mkdata.py $keys
  echo ""
  echo "--------------------------------------------------"
  ./flogdel2.py
  if [ $? == 1 ]; then
    echo " failed!"
    exit 1
  fi
  rm -f fa_inc.py fa_inc.pyc
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
