#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
keys=250000
lengths=10
while [ $cnt -le $COUNT ]; do
 echo "$keys keys for #$cnt ..."
 ./words $keys $lengths > inputshl
 echo "sorting..."
 sort -f inputshl > inputshl.srt
 echo "reverse sorting..."
 sort -fr inputshl > inputshl.rev.srt
 echo "running #$cnt ..."
 ./flogshl3 $keys inputshl inputshl.srt inputshl.rev.srt
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
