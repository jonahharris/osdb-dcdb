#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
keys=250000
lengths=24
while [ $cnt -le $COUNT ]; do
 echo "$keys keys for #$cnt ..."
 ./words $keys $lengths > inputshl
 echo "sorting..."
 sort inputshl > inputshl.srt
 echo "running #$cnt ..."
 ./flogshsort inputshl $keys > inputshl.out
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
 echo "difference (should be nothing)..."
 diff inputshl.out inputshl.srt
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
