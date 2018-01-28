#
LD_LIBRARY_PATH="../../lib"
if [ "x$COUNT" == "x" ]; then
 COUNT=25
fi

cnt=1
while [ $cnt -le $COUNT ]; do
 echo "running #$cnt ..."
 ./flogrmshl
 if [ $? != 0 ]; then
  echo " failed!"
  exit 1
 fi
 echo "Passed!"

 cnt=`expr $cnt + 1`
done
