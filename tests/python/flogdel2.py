#!/usr/bin/python2.5

# Load the python module.
import sys
import os
sys.path[0] = ''
os.chdir ("../../lib")
import cdbpy
os.chdir ("../tests/python")

# Lock and load.
fdel2 = open ("flogdel2.df", "w", 0644)
file.write (fdel2, '// flogdel2.df\n')
file.write (fdel2, 'create table "flogdel2.db"\n')
file.write (fdel2, '  info "Table for flogdel2.py"\n')
file.write (fdel2, '{\n')
file.write (fdel2, '  "LName" char (65);\n')
file.write (fdel2, '  "SSNumber" char (9);\n')
file.write (fdel2, '  "FName" char (35);\n')
file.write (fdel2, '  "StreetAddress" char (80);\n')
file.write (fdel2, '  "City" char (50);\n')
file.write (fdel2, '  "State" char (2);\n')
file.write (fdel2, '  "Zip" char (10);\n')
file.write (fdel2, '  "AnnualSalary" number (10:2);\n')
file.write (fdel2, '  "HomeOwner" logical;\n')
file.write (fdel2, '  "DateApplied" date;\n')
file.write (fdel2, '  "LastUpdated" time;\n')
file.write (fdel2, '} indexed {\n')
file.write (fdel2, '  idx "fd2_ssnidx" 256:case:unique "SSNumber";\n')
file.write (fdel2, '  idx "fd2_lnidx" 512:case:dup "LName";\n')
file.write (fdel2, '};')
file.write (fdel2, '\n\n')

file.close (fdel2)

if (os.access ("flogdel2.db", os.F_OK)) :
  os.remove ("flogdel2.db")

if (os.access ("flogdel2.db.LCK", os.F_OK)) :
  os.remove ("flogdel2.db.LCK")

if (os.access ("fd2_ssnidx.inx", os.F_OK)) :
  os.remove ("fd2_ssnidx.inx")

if (os.access ("fd2_ssnidx.idx", os.F_OK)) :
  os.remove ("fd2_ssnidx.idx")

if (os.access ("fd2_lnidx.inx", os.F_OK)) :
  os.remove ("fd2_lnidx.inx")

if (os.access ("fd2_lnidx.idx", os.F_OK)) :
  os.remove ("fd2_lnidx.idx")

#
# Create the table and indexes
#
status = cdbpy.dbcreate ("flogdel2.df")
if (status != 0) :
  print "%s %s" % ("\n\n***Error: dbcreate flogdel2.df:", cdbpy.dbgeterror())
  sys.exit (1)

os.remove ("flogdel2.df")

table = cdbpy.dbopen ("flogdel2.db")
if (table == '') :
  print "%s %s" % ("\n\n***Error: dbopen flogdel2.db:", cdbpy.dbgeterror())
  sys.exit (1)

print ("Parsing testAdd.txt")

s = open ("testAdd.txt").read()
a1 = []
a1 = s.split()

ssn_dups = []

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
      ssn_dups.append (a2[1])
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

print "Finished adding to %s: %d in %6.4f seconds" % (table, num, t2-t1)

cdbpy.dbclose (table)

table = cdbpy.dbopen ("flogdel2.db")
if (table == '') :
  print "Opening flogdel2.db the second time: %s" % (cdbpy.dbgeterror())
  sys.exit (1)

#
# Now, traverse through the table and insure that the data is consistent with
# what was actually entered.
#

t1 = 0.0
t1 = cdbpy.dbtime ()
counter = 0
cdbpy.dbcurrent (table, "f1_ssnidx")
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
    if (a2[1] in ssn_dups): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "LName",
  SSNumber = cdbpy.dbshow (table, "SSNumber")
  status = cmp (SSNumber, a2[1])
  if (status != 0) :
    if (a2[1] in ssn_dups): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "SSNumber",
  FName = cdbpy.dbshow (table, "FName")
  status = cmp (FName, a2[2])
  if (status != 0) :
    if (a2[1] in ssn_dups): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "FName",
  StreetAddress = cdbpy.dbshow (table, "StreetAddress")
  status = cmp (StreetAddress, a2[3])
  if (status != 0) :
    if (a2[1] in ssn_dups): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "StreetAddress",
  City = cdbpy.dbshow (table, "City")
  status = cmp (City, a2[4])
  if (status != 0) :
    if (a2[1] in ssn_dups): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "City",
  Zip = cdbpy.dbshow (table, "Zip")
  status = cmp (Zip, a2[6])
  if (status != 0) :
    if (a2[1] in ssn_dups): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "Zip",
  AnnualSalary = cdbpy.dbshow (table, "AnnualSalary")
  status = cmp (AnnualSalary, a2[7])
  if (status != 0) :
    if (a2[1] in ssn_dups): continue
    if not header :
      print "***Warning: Record out of sync ",
      header = 1
    print "AnnualSalary",
  del a2

t2 = 0.0
t2 = cdbpy.dbtime ()
num = cdbpy.dbnumrecs (table)

print "\nFinished verifying to %s: %d in %6.4f seconds" % (table, num, t2-t1)

ssn_list = []
lname_list = []
tpass = 0
i = 0
execfile ("fa_inc.py")
while (i < 4) :
  tpass += 1
  i += 1
  counter = 0
  step = 5 - i
  t1 = 0.0
  t1 = cdbpy.dbtime ()
  print "Pass %s" % (tpass)
  cdbpy.dbhead (table)
  j = 0
  while (j < delNumber) :
    j += 1
    counter += 1
    k = 0
    while (k < step) :
      k += 1
      cdbpy.dbnext (table)
      if (cdbpy.dbiseof (table)) : break
    if (cdbpy.dbiseof (table)) : break
    cdbpy.dbretrieve (table)
    this_lname = cdbpy.dbshow (table, "LName")
    this_ssn = cdbpy.dbshow (table, "SSNumber")
    ssn_list.append (this_ssn)
    lname_list.append (this_lname)
    cdbpy.dbdel (table)
  t2 = 0.0
  t2 = cdbpy.dbtime ()
  print "%d items deleted from %s in %f seconds" % (j, table, t2-t1)
  print "Packing %s" % (table)
  t1 = 0.0
  t1 = cdbpy.dbtime ()
  status = cdbpy.dbpack (table)
  if (status == -1) :
    print "\n\n***Error: packing %s: %s\n" % (table, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  cdbpy.dbclose (table)
  table = cdbpy.dbopen ("flogdel2.db")
  if (table == '') :
    print "\n\n***Error: couldn't open flogdel2.db second time: %s\n" % (cdbpy.dbgeterror())
    sys.exit (1)
  t2 = 0.0
  t2 = cdbpy.dbtime ()
  num_recs = cdbpy.dbnumrecs (table)
  print "Packed %s in %f seconds - %d records left in the table" % (table, t2-t1, num_recs)
  t1 = cdbpy.dbtime ()
  for ssn in ssn_list :
    if (cdbpy.dbsearchexact (table, ssn)) :
      print "***Warning: Could search for and retrieve deleted SSN: %s" % (ssn)
  cdbpy.dbcurrent (table, "fd2_lnidx")
  for lname in lname_list :
    if (cdbpy.dbsearchexact (table, lname)) :
      print "Could find deleted last name - probably a dup: %s" % (lname)
  t2 = cdbpy.dbtime ()
  numItems = len (ssn_list)
  print "Searched %d in %f seconds.\n" % (numItems, t2-t1)

print "There were %d records left after all in %s\n" % (cdbpy.dbnumrecs (table), table)

cdbpy.dbexit ()
sys.exit (0)

cdbpy.dbclose (table)

sys.exit (0)
