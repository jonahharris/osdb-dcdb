#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
if [ -e table.db ]; then
 rm -f table.db
 rm -f SSNumberIndex.i?x
 rm -f LNameIndex.i?x
 rm -f NamesIndex.i?x
fi
while [ $cnt -le $COUNT ]; do
 echo "5000 keys for #$cnt ..."
 rm -f testAdd.txt
 ./mk3data
 /usr/bin/cut -d: -f1,2 testAdd.txt |/bin/sort -t: -k1 >lname.srt
 /usr/bin/cut -d: -f2 testAdd.txt >tmpfile1
 /usr/bin/cut -d: -f1 testAdd.txt >tmpfile2
 /usr/bin/paste -d: tmpfile1 tmpfile2 |/bin/sort -t: -k1 >ssn.srt
 rm -f tmpfile1 tmpfile2
 echo "running #$cnt ..."
 ./flogridx lname.srt ssn.srt
 if [ $? != 0 ]; then
  echo " failed!"
  echo "lname,ssn producing the failure is left in \"lname.srt\" and \"ssn.srt\""
  exit 1
 fi
 echo ""
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
