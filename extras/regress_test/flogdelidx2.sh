#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
keys=25000
lengths=10
while [ $cnt -le $COUNT ]; do
  echo "$keys keys for #$cnt ..."
  ./words $keys $lengths > input
  echo "running #$cnt ..."
  ./flogdelidx2 input
  if [ $? != 0 ]; then
    echo " failed!"
    echo "input producing the failure is left in \"input\""
    exit 1
  fi
  echo "Passed!"

  cnt=`expr $cnt + 1`
done
