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
  echo "50000 keys for #$cnt ..."
  ./mksdata
  /usr/bin/cut -d: -f1,2 testAdd.txt |/bin/sort -t: -k1 >lname.srt
  /usr/bin/cut -d: -f2 testAdd.txt >tmpfile1
  /usr/bin/cut -d: -f1 testAdd.txt >tmpfile2
  /usr/bin/paste -d: tmpfile1 tmpfile2 |/bin/sort -t: -k1 >ssn.srt
  rm tmpfile1 tmpfile2
  echo "running #$cnt ..."
  ./flogdel lname.srt ssn.srt
  if [ $? != 0 ]; then
    echo " failed!"
    echo "lname,ssn producing the failure is left in \"lname.srt\" and \"ssn.srt\""
    exit 1
  fi
  echo ""
  echo "Passed!"

  cnt=`expr $cnt + 1`
done
