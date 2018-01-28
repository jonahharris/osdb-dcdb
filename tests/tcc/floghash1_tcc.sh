
cnt=1
keys=500000
length=10
endhere=100

if [ "$1" != "" ]; then
  endhere="$1"
fi

while [ $cnt -le $endhere ]; do
 echo "$keys keys for #$cnt ..."
 tcc -I../../include -L../../lib "-run -lcdbtcl" ./words.c $keys $length > inputshl
 tcc -I../../include -L../../lib "-run -lcdbtcl" ./rough_sort.c inputshl > inputshl.srt
 echo "running #$cnt ..."
 tcc -I../../include -L../../lib "-run -lcdbtcl" ./floghash1.c inputshl.srt
 if [ $? != 0 ]; then
  echo " failed!"
  echo "inputshl producing the failure is left in \"inputshl\""
  exit 1
 fi
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
