#!/usr/bin/python2.5

# Load the python module.
import sys
import os
sys.path[0] = ''
os.chdir ("../../lib")
import cdbpy
os.chdir ("../tests/python")

# Lock and load.
fadd1 = open ("flogreindex1.df", "w", 0644)
file.write (fadd1, "// flogreindex1.df\n")
file.write (fadd1, 'create table "flogreindex1.db"\n')
file.write (fadd1, '  info "Table for flogreindex1.py"\n')
file.write (fadd1, "{\n")
file.write (fadd1, '  "LName" char (65);\n')
file.write (fadd1, '  "SSNumber" char (9);\n')
file.write (fadd1, '  "FName" char (35);\n')
file.write (fadd1, '  "StreetAddress" char (80);\n')
file.write (fadd1, '  "City" char (50);\n')
file.write (fadd1, '  "State" char (2);\n')
file.write (fadd1, '  "Zip" char (10);\n')
file.write (fadd1, '  "AnnualSalary" number (10:2);\n')
file.write (fadd1, '  "HomeOwner" logical;\n')
file.write (fadd1, '  "DateApplied" date;\n')
file.write (fadd1, '  "LastUpdated" time;\n')
file.write (fadd1, '} indexed {\n')
file.write (fadd1, '  idx "fr1_ssnidx" 256:case:unique "SSNumber";\n')
file.write (fadd1, '  idx "fr1_lnidx" 512:case:dup "LName";\n')
file.write (fadd1, '  midx "fr1_lfnidx" 512:case "LName", "FName";\n')
file.write (fadd1, '};')
file.write (fadd1, "\n\n")

file.close (fadd1)

if (os.access ("flogreindex1.db", os.F_OK)) :
  os.remove ("flogreindex1.db")

if (os.access ("flogreindex1.db.LCK", os.F_OK)) :
  os.remove ("flogreindex1.db.LCK")

if (os.access ("fr1_ssnidx.inx", os.F_OK)) :
  os.remove ("fr1_ssnidx.inx")

if (os.access ("fr1_ssnidx.idx", os.F_OK)) :
  os.remove ("fr1_ssnidx.idx")

if (os.access ("fr1_lnidx.inx", os.F_OK)) :
  os.remove ("fr1_lnidx.inx")

if (os.access ("fr1_lnidx.idx", os.F_OK)) :
  os.remove ("fr1_lnidx.idx")

if (os.access ("fr1_lfnidx.inx", os.F_OK)) :
  os.remove ("fr1_lfnidx.inx")

if (os.access ("fr1_lfnidx.idx", os.F_OK)) :
  os.remove ("fr1_lfnidx.idx")

#
# Create the table and indexes
#
status = cdbpy.dbcreate ("flogreindex1.df")
if (status != 0) :
  print "%s %s" % ("\n\n***Error: dbcreate flogreindex1.df:", cdbpy.dbgeterror())
  sys.exit (1)

os.remove ("flogreindex1.df")

table = cdbpy.dbopen ("flogreindex1.db")
if (table == '') :
  print "%s %s" % ("\n\n***Error: dbopen flogreindex1.db:", cdbpy.dbgeterror())
  sys.exit (1)

print ("Parsing testAdd.txt")

s = open ("testAdd.txt").read()
a1 = []
a1 = s.split()

dupList = []

t1 = 0.0
t1 = cdbpy.dbtime ()
counter = 0
for ln in a1 :
  counter += 1
  if (counter % 1000 == 0) :
    sys.stdout.write('.')
    sys.stdout.flush()
  a2 = []
  a2 = ln.split(':')
  status = cdbpy.dbsetchar (table, "LName", a2[0])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table LName ", a2[0], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "SSNumber", a2[1])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table SSNumber ", a2[1], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "FName", a2[2])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table FName ", a2[2], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "StreetAddress", a2[3])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table StreetAddress ", a2[3], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "City", a2[4])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table City ", a2[4], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "State", a2[5])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table State ", a2[5], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "Zip", a2[6])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table Zip ", a2[6], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetnum (table, "AnnualSalary", float(a2[7]))
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table AnnualSalary ", a2[7], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetlog (table, "HomeOwner", a2[8])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table HomeOwner ", a2[8], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsetdate (table, "DateApplied", "0")
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table DateApplied ", 0, cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbsettime (table, "LastUpdated", "0")
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table FName ", 0, cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  status = cdbpy.dbadd (table)
  if (status == -1) :
    terr = cdbpy.dbgeterror()
    status = cmp (terr, "adding record: unique index constraint violated")
    if not status :
      print "***Warning: Duplicate SSN at line %d" % (counter)
      dupList.append (a2[1])
      continue
    print "%s:%s" % \
        ("\n\n***Error: dbadd table ", cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  del a2

sys.stdout.write ("\n")
t2 = 0.0
t2 = cdbpy.dbtime ()
num = cdbpy.dbnumrecs (table)

print "\nFinished adding to %s: %d in %6.4f seconds" % (table, num, t2-t1)

#
# Close the table and reindex.
#
print "\nReindexing table %s..." % (table)

t1 = 0.0
t1 = cdbpy.dbtime ()

cdbpy.dbclose (table)
status = cdbpy.dbreindex ("flogreindex1.db")
if (status == -1) :
  print "***Error: dbreindex (flogreindex1.db) failed: %s\n" % (cdbpy.dbgeterror())
  sys.exit (1)

t2 = 0.0
t2 = cdbpy.dbtime ()

print "\nReindex of table %s in %f seconds\n" % (table, t2-t1)

table = cdbpy.dbopen ("flogreindex1.db")
if (table == '') :
  print "%s %s" % ("\n\n***Error: dbopen flogreindex1.db:", cdbpy.dbgeterror())
  sys.exit (1)

#
# Now, traverse through the table and insure that the data is consistent with
# what was actually entered.
#

t1 = 0.0
t1 = cdbpy.dbtime ()
counter = 0
cdbpy.dbcurrent (table, "fr1_ssnidx")
status = cdbpy.dbiseof (table)
if (status == 1) :
  print "There are no records in %s" % (table)
  cdbpy.dbexit ()
  sys.exit (1)

for ln in a1 :
  counter += 1
  if (counter % 1000 == 0) :
    sys.stdout.write('.')
    sys.stdout.flush()
  a2 = []
  a2 = ln.split(':')
  status = cdbpy.dbsearchexact (table, a2[1])
  if (status == 0) :
    print "***Warning: couldn't find %s in the database: line %d" % (a2[0], counter)
    continue
  status = cdbpy.dbretrieve (table)
  if (status == -1) :
    print "\n\n***Error: Could not retrieve from %s: %s" % (table, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  header = 0
  LName = cdbpy.dbshow (table, "LName")
  status = cmp (LName, a2[0])
  if (status != 0) :
    if (a2[1] in dupList): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "LName",
  SSNumber = cdbpy.dbshow (table, "SSNumber")
  status = cmp (SSNumber, a2[1])
  if (status != 0) :
    if (a2[1] in dupList): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "SSNumber",
  FName = cdbpy.dbshow (table, "FName")
  status = cmp (FName, a2[2])
  if (status != 0) :
    if (a2[1] in dupList): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "FName",
  StreetAddress = cdbpy.dbshow (table, "StreetAddress")
  status = cmp (StreetAddress, a2[3])
  if (status != 0) :
    if (a2[1] in dupList): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "StreetAddress",
  City = cdbpy.dbshow (table, "City")
  status = cmp (City, a2[4])
  if (status != 0) :
    if (a2[1] in dupList): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "City",
  Zip = cdbpy.dbshow (table, "Zip")
  status = cmp (Zip, a2[6])
  if (status != 0) :
    if (a2[1] in dupList): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "Zip",
  AnnualSalary = cdbpy.dbshow (table, "AnnualSalary")
  status = cmp (AnnualSalary, a2[7])
  if (status != 0) :
    if (a2[1] in dupList): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "AnnualSalary",
  del a2

del dupList

t2 = 0.0
t2 = cdbpy.dbtime ()
num = cdbpy.dbnumrecs (table)

print "\nFinished verifying to %s: %d in %6.4f seconds" % (table, num, t2-t1)

cdbpy.dbclose (table)

sys.exit (0)
