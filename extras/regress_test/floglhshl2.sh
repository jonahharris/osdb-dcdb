#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
  COUNT=25
fi

cnt=1
keys=150000
lengths=35
while [ $cnt -le $COUNT ]; do
  echo "$keys keys for #$cnt ..."
  ./words $keys $lengths > inputshl
  echo "sorting..."
  sort inputshl > inputshl.srt
  echo "reverse sorting..."
  sort -r inputshl > inputshl.rev.srt
  echo "running #$cnt ..."
  ./floglhshl inputshl inputshl.srt inputshl.rev.srt
  if [ $? != 0 ]; then
    echo " failed!"
    echo "inputshl producing the failure is left in \"inputshl\""
    exit 1
  fi
  echo "Passed!"

  cnt=`expr $cnt + 1`
done
