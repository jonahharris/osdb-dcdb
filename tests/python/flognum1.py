#!/usr/bin/python2.5

# Load the python module.
import sys
import os
sys.path[0] = ''
os.chdir ("../../lib")
import cdbpy
os.chdir ("../tests/python")

# Lock and load.
if (len (sys.argv) == 2) :
  numIterations = int (sys.argv[1])
  if (numIterations < 2) : numIterations = 2
  if (numIterations > 100) : numIterations = 100
else :
  print "Usage: flognum1.py numIterations"
  sys.exit (1)

print "\n*** Exercise dbnumadd ...\n"
i = 0
while (i < numIterations) :
  i += 1
  sys.stdout.write ("Iteration ")
  sys.stdout.write (str(i))
  sys.stdout.write (" ")
  val = cdbpy.dbtestnumber (10, 0)
  val += ".00"
  counter = 0
  while (float(val) > 0.00) :
    counter += 1
    if (counter % 10000 == 0) :
      sys.stdout.write (".")
      sys.stdout.flush ()
    val1 = val
    num = cdbpy.dbtestnumber (5, 0)
    sub = "-" + num
    val = cdbpy.bcnumadd (val, sub, 2)
    val1f = float (val1) + float (sub)
    val1 = str (val1f)
    status = cdbpy.bcnumcompare (val, val1, 2)
    if (status != 0) :
      print "%s and %s are not equal and they should be" % (val, val1)
  print "\ncounter = %d\n" % (counter)

print "\n*** Exercise dbnumsub ...\n"
i = 0
while (i < numIterations) :
  i += 1
  sys.stdout.write ("Iteration ")
  sys.stdout.write (str(i))
  sys.stdout.write (" ")
  val = cdbpy.dbtestnumber (10, 0)
  val += ".00"
  counter = 0
  while (float(val) > 0.00) :
    counter += 1
    if (counter % 10000 == 0) :
      sys.stdout.write (".")
      sys.stdout.flush ()
    val1 = val
    num = cdbpy.dbtestnumber (5, 0)
    val = cdbpy.bcnumsub (val, num, 2)
    val1f = float (val1) - float (num)
    val1 = str (val1f)
    status = cdbpy.bcnumcompare (val, val1, 2)
    if (status != 0) :
      print "%s and %s are not equal and they should be" % (val, val1)
  print "\ncounter = %d\n" % (counter)

print "\n*** Exercise dbnumdivide ...\n"
i = 0
while (i < numIterations) :
  i += 1
  sys.stdout.write ("Iteration ")
  sys.stdout.write (str(i))
  sys.stdout.write (" ")
  val = cdbpy.dbtestnumber (50, 0)
  val += ".00"
  counter = 0
  while (float(val) > 2.00) :
    counter += 1
    if (counter % 10000 == 0) :
      sys.stdout.write (".")
      sys.stdout.flush ()
    num = cdbpy.dbtestnumber (2, 0)
    val = cdbpy.bcnumdivide (val, num, 2)
  print "\ncounter = %d\n" % (counter)

print "\n*** Exercise dbnummultiply ...\n"
i = 0
while (i < numIterations) :
  i += 1
  sys.stdout.write ("Iteration ")
  sys.stdout.write (str(i))
  sys.stdout.write (" ")
  val = cdbpy.dbtestnumber (2, 0)
  val += ".00"
  topval = cdbpy.dbtestnumber (60, 0)
  topval += ".00"
  counter = 0
  while (float(val) < float (topval)) :
    counter += 1
    if (counter % 10000 == 0) :
      sys.stdout.write (".")
      sys.stdout.flush ()
    num = cdbpy.dbtestnumber (2, 0)
    val = cdbpy.bcnummultiply (val, num, 2)
  print "\ncounter = %d\n" % (counter)

print "\n*** Exercise dbnumraise ...\n"
i = 0
while (i < numIterations) :
  i += 1
  sys.stdout.write ("Iteration ")
  sys.stdout.write (str(i))
  sys.stdout.write (" ")
  val = str (i+1)
  val += ".00"
  topval = cdbpy.dbtestnumber (50, 0)
  topval += ".00"
  counter = 0
  while (float (val) < float (topval)) :
    counter += 1
    if (counter % 10000 == 0) :
      sys.stdout.write (".")
      sys.stdout.flush ()
    num = cdbpy.dbtestnumber (2, 0)
    val = cdbpy.bcnumraise (val, "2.00", 2)
  print "\ncounter = %d\n" % (counter)

cdbpy.bcnumuninit ()
sys.exit (0)
