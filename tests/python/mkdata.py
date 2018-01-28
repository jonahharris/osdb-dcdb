#!/usr/bin/python2.5

# Load the python module.
import sys
import os
sys.path[0] = ''
os.chdir ("../../lib")
import cdbpy
os.chdir ("../tests/python")

numItems = 0
if len (sys.argv) == 2:
  numItems = int(sys.argv[1])
else:
  numItems = 5000

fp = open ("testAdd.txt", "w", 0644)

i = 0
while (i < numItems):
  LName = cdbpy.dbtestupperstring (0, 64)
  file.write (fp, LName)
  file.write (fp, ":")
  SSNumber = cdbpy.dbtestnumber (9, 0)
  file.write (fp, SSNumber)
  file.write (fp, ":")
  FName = cdbpy.dbtestupperstring (0, 34)
  file.write (fp, FName)
  file.write (fp, ":")
  StreetAddress = cdbpy.dbteststring (0, 34)
  file.write (fp, StreetAddress)
  file.write (fp, ":")
  City = cdbpy.dbteststring (45, 0)
  file.write (fp, City)
  file.write (fp, ":")
  State = "NM"
  file.write (fp, State)
  file.write (fp, ":")
  Zip = cdbpy.dbtestnumber (10, 0)
  file.write (fp, Zip)
  file.write (fp, ":")
  AnnualSalary = cdbpy.dbtestnumber (7, 0)
  file.write (fp, AnnualSalary)
  file.write (fp, ".00")
  file.write (fp, ":")
  HomeOwner = "Y"
  file.write (fp, HomeOwner)
  file.write (fp, ":")
  DateApplied = cdbpy.dbtestnumber (8, 0)
  file.write (fp, DateApplied)
  file.write (fp, ":")
  LastUpdated = cdbpy.dbteststring (26, 0)
  file.write (fp, LastUpdated)
  file.write (fp, "\n")
  i += 1

file.close (fp)
sys.exit (0)

