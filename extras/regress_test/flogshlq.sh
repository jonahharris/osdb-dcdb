#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
keys=500000
lengths=35
while [ $cnt -le $COUNT ]; do
 ./words $keys $lengths > inputshl
 echo "$keys keys for #$cnt ..."
 echo "" >>inputshl
 echo "sorting..."
 sort inputshl > inputshl.srt
 echo "reverse sorting..."
 sort -r inputshl > inputshl.rev.srt
 echo "running #$cnt ..."
 ./flogshlq $keys inputshl inputshl.srt inputshl.rev.srt
 if [ $? != 0 ]; then
  echo " failed!"
  echo "input producing the failure is left in \"inputshl\""
  exit 1
 fi
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
