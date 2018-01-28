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
 ./rough_sort $keys $length inputshl > inputshl.srt
 echo "running #$cnt ..."
 ./flogthrd1 inputshl.srt
 sort inputshl > inputshl.rsrt
 echo "diff between sorted output via qsort and flogthrd1...(nothing is good here)"
 echo ""
 diff inputshl.rsrt inputshl.out
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
 rm -f inputshl inputshl.srt
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
