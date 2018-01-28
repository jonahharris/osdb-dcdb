#!/bin/bash
#Script that calculates an average of the data items in a colon delimited text
# file.  It takes 3 arguments:
#
# $1 - the field (1,2,3,etc).
# $2 The file to process.
# $3 The number of data items (number of lines in the file).
#
# The result is average of the data items in the specified field.
#
calc_field_usage()
{
  echo "Usage: $0 <fldnum> <file> <numlines>"
  echo " where fldnum = number of field to process,"
  echo " file is the file to process, and"
  echo " numlines is the number of lines to process from the file."
  echo ""
  exit 1
}
if [ "$1x" == "x" ]; then
  calc_field_usage
fi

if [ "$2x" == "x" ]; then
  calc_field_usage
fi
echo "scale = 6" > run_calc.bc
echo "total = 0" >> run_calc.bc
for i in `cut -f$1 -d: $2`; do
    echo "total += $i" >> run_calc.bc
  done
echo "total /= $3" >> run_calc.bc
echo "total /= 1" >> run_calc.bc
echo "total" >> run_calc.bc
echo "quit" >> run_calc.bc
bc -q run_calc.bc
rm -f run_calc.bc
exit 0
