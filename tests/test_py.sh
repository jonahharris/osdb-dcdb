#!/bin/sh

if [ "$1" == "" ]; then
  echo "Usage: $0 num_items"
  echo "  where num_items is the number of iterations to run (i.e., 50)"
  echo "  of each test."
  exit 1
fi

cd python

echo ""
echo "Doing $1 iterations..."
echo ""

echo "Executing flogadd1.sh: `date +%Y%m%d@%T`"
bash flogadd1.sh $1
if [ $? != 0 ]; then
	echo "flogadd1.sh failed"
	exit 1
fi
echo "Finished flogadd1.sh: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogadd2.sh: `date +%Y%m%d@%T`"
bash flogadd2.sh $1
if [ $? != 0 ]; then
	echo "flogadd2.sh failed"
	exit 1
fi
echo "Finished flogadd2.sh: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogdel1.sh: `date +%Y%m%d@%T`"
bash flogdel1.sh $1
if [ $? != 0 ]; then
	echo "flogdel1.sh failed"
	exit 1
fi
echo "Finished flogdel1.sh: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogdel2.sh: `date +%Y%m%d@%T`"
bash flogdel2.sh $1
if [ $? != 0 ]; then
	echo "flogdel2.sh failed"
	exit 1
fi
echo "Finished flogdel2.sh: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogdel3.sh: `date +%Y%m%d@%T`"
bash flogdel3.sh $1
if [ $? != 0 ]; then
	echo "flogdel3.sh failed"
	exit 1
fi
echo "Finished flogdel3.sh: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogreindex1.sh: `date +%Y%m%d@%T`"
bash flogreindex1.sh $1
if [ $? != 0 ]; then
	echo "flogreindex1.sh failed"
	exit 1
fi
echo "Finished flogreindex1.sh: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogenc1.sh: `date +%Y%m%d@%T`"
bash flogenc1.sh $1
if [ $? != 0 ]; then
	echo "flogenc1.sh failed"
	exit 1
fi
echo "Finished flogenc1.sh: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogupd1: `date +%Y%m%d@%T`"
bash flogupd1.sh $1
if [ $? != 0 ]; then
	echo "flogupd1 failed"
	exit 1
fi
echo "Finished flogupd1: `date +%Y%m%d@%T`"
echo ""
echo ""

exit 0

# end here until finished

echo "Executing flognum1: `date +%Y%m%d@%T`"
bash flognum1.sh $1
if [ $? != 0 ]; then
	echo "flognum1 failed"
	exit 1
fi
echo "Finished flognum1: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing floghash1: `date +%Y%m%d@%T`"
bash floghash1.sh $1
if [ $? != 0 ]; then
	echo "floghash1 failed"
	exit 1
fi
echo "Finished floghash1: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flogthrd1: `date +%Y%m%d@%T`"
bash flogthrd1.sh $1
if [ $? != 0 ]; then
	echo "flogthrd1 failed"
	exit 1
fi
echo "Finished floghash1: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Executing flognum2: `date +%Y%m%d@%T`"
bash flognum2.sh $1
if [ $? != 0 ]; then
	echo "flognum2 failed"
	exit 1
fi
echo "Finished flognum2: `date +%Y%m%d@%T`"
echo ""
echo ""

echo "Everything completed successfully."

