cnt=1
keys=500000
length=10
endhere=100

if [ "$1" != "" ]; then
  endhere="$1"
fi

echo "Testing threaded sort..."
echo ""

while [ $cnt -le $endhere ]; do
 echo "$keys keys for #$cnt ..."
 tcc -I../../include -L../../lib "-run -lcdbtcl" ./words.c $keys $length > inputshl
 tcc -I../../include -I../../lib -L../../lib "-run" ./rough_sort.c $keys $length inputshl > inputshl.srt
 echo "running #$cnt ..."
 tcc -I../../include -L../../lib "-run -lcdbtcl" ./flogthrd1.c inputshl.srt
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
 rm -f inputshl inputshl.srt
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
