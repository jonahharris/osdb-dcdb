#

cnt=1
keys=10
incr=1
endhere=20
if [ "$1" != "" ]; then
  endhere="$1"
fi

uname_s=`uname -s`
OS="Unknown"
if [ "$uname_s" == "Linux" ]; then
  OS="Linux"
fi
if [ "$uname_s" == "SunOS" ]; then
  OS="Solaris"
fi
if [ "$OS" == "Unknown" ]; then
  uname_o=`uname -o`
  if [ "$uname_o" == "Cygwin" ]; then
    OS="Cygwin"
  fi
fi

if [ "$OS" == "Linux" ]; then
  TCLSH=tclsh
fi
if [ "$OS" == "Solaris" ]; then
  TCLSH=tclsh8.4
fi
if [ "$OS" == "Cygwin" ]; then
  TCLSH=tclsh
fi

while [ 1  = 1 ]; do
  echo "Running #$cnt"
  echo "$keys iterations"
  echo ""
  echo "--------------------------------------------------"
  $TCLSH flognum1.tcl $keys
  if [ $? == 1 ]; then
    echo " failed!"
    exit 1
  fi
  if [ $cnt == $endhere ]; then
    echo "Finished flogcont1."
    exit 0
  fi
  keys=`expr $keys + $incr`
  echo "--------------------------------------------------"
  echo ""
  echo "Passed"
  echo ""

  cnt=`expr $cnt + 1`
done
