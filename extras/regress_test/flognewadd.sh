#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
keys=2500
lengths=10
if [ -e testIndex.idx ]; then
	rm -f testIndex.idx
	rm -f testIndex.inx
fi
while [ $cnt -le $COUNT ]; do
  echo "$keys keys for #$cnt ..."
  ./words $keys $lengths > input
  ./mkdups input inputdup.1 3
  ./mkdups inputdup.1 input 2
  echo "running #$cnt ..."
  ./flognewadd input
  if [ $? != 0 ]; then
    echo " failed!"
    echo "input producing the failure is left in \"input\""
    exit 1
  fi
  echo "Passed!"

  cnt=`expr $cnt + 1`
done
