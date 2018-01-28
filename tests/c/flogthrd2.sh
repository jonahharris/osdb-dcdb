cnt=1
keys=500000
length=35
endhere=100

if [ "$1" != "" ]; then
  endhere="$1"
fi

echo "Testing threaded sort..."
echo ""

while [ $cnt -le $endhere ]; do
 echo "$keys keys for #$cnt ..."
 ./words $keys $length > inputshl
 echo "running #$cnt ..."
 ./flogthrd2 inputshl $keys
# sort inputshl > inputshl.srt
# echo "diff between sorted output via qsort and flogthrd1...(nothing is good here)"
# echo ""
# diff inputshl.srt inputshl.out
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
# rm -f inputshl inputshl.srt
 rm -f inputshl
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
