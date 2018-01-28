cnt=1
keys=500000
length=10
endhere=100

if [ "$1" != "" ]; then
  endhere="$1"
fi

while [ $cnt -le $endhere ]; do
 echo "$keys keys for #$cnt ..."
 ./words $keys $length > inputshl
 ./rough_sort inputshl > inputshl.srt
 echo "running #$cnt ..."
 ./floghash1 inputshl.srt
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
