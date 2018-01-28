#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
keys=500000
lengths=35
while [ $cnt -lt $COUNT ]; do
 echo "$keys keys for #$cnt ..."
 ./words $keys $lengths > inputshl
 # Uncomment the following if you want to test dups.
 echo "sorting..."
 sort inputshl |uniq >  inputshl.srt
 echo "reverse sorting..."
 sort -r inputshl.srt > inputshl.rev.srt
 echo "running #$cnt ..."
 ./flogshl inputshl inputshl.srt inputshl.rev.srt
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
 sleep 5
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
