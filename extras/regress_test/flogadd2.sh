#

LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
while [ $cnt -le $COUNT ]; do
  if [ -e table.db ]; then
    rm -f table.db
    rm -f SSNumberIndex.i?x
    rm -f LNameIndex.i?x
  fi
  echo "100000 keys for #$cnt ..."
  ./mkdata
  echo "running #$cnt ..."
  ./flogadd2
  if [ $? != 0 ]; then
    echo " failed!"
    echo "inputtre producing the failure is left in \"inputtre\""
    exit 1
  fi
  echo ""
  echo "Passed!"

  cnt=`expr $cnt + 1`
done
