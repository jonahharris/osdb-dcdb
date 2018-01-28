#!/usr/bin/python2.5

# Load the python module.
import sys
import os
sys.path[0] = ''
os.chdir ("../../lib")
import cdbpy
os.chdir ("../tests/python")

# Lock and load.
fdel2 = open ("flogdel3.df", "w", 0644)
file.write (fdel2, '// flogdel3.df\n')
file.write (fdel2, 'create table "flogdel3.db"\n')
file.write (fdel2, '  info "Table for flogdel3.py"\n')
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
file.write (fdel2, '  idx "fd3_ssnidx" 256:case:unique "SSNumber";\n')
file.write (fdel2, '  idx "fd3_lnidx" 512:case:dup "LName";\n')
file.write (fdel2, '  midx "fd3_lfname" 512:case "LName", "FName";\n')
file.write (fdel2, '};')
file.write (fdel2, '\n\n')

file.close (fdel2)

if (os.access ("flogdel3.db", os.F_OK)) :
  os.remove ("flogdel3.db")

if (os.access ("flogdel3.db.LCK", os.F_OK)) :
  os.remove ("flogdel3.db.LCK")

if (os.access ("fd3_ssnidx.inx", os.F_OK)) :
  os.remove ("fd3_ssnidx.inx")

if (os.access ("fd3_ssnidx.idx", os.F_OK)) :
  os.remove ("fd3_ssnidx.idx")

if (os.access ("fd3_lnidx.inx", os.F_OK)) :
  os.remove ("fd3_lnidx.inx")

if (os.access ("fd3_lnidx.idx", os.F_OK)) :
  os.remove ("fd3_lnidx.idx")

if (os.access ("fd3_lfname.inx", os.F_OK)) :
  os.remove ("fd3_lfname.inx")

if (os.access ("fd3_lfname.idx", os.F_OK)) :
  os.remove ("fd3_lfname.idx")

#
# Create the table and indexes
#
status = cdbpy.dbcreate ("flogdel3.df")
if (status != 0) :
  print "%s %s" % ("\n\n***Error: dbcreate flogdel3.df:", cdbpy.dbgeterror())
  sys.exit (1)

os.remove ("flogdel3.df")

table = cdbpy.dbopen ("flogdel3.db")
if (table == '') :
  print "%s %s" % ("\n\n***Error: dbopen flogdel3.db:", cdbpy.dbgeterror())
  sys.exit (1)

print ("Parsing testAdd.txt")

s = open ("testAdd.txt").read()
a1 = []
a1 = s.split()

ssn_dups = []

new_lname = []
new_ssn = []
new_lfname = []

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
  new_lname.append (a2[0])
  status = cdbpy.dbsetchar (table, "SSNumber", a2[1])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table SSNumber ", a2[1], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  new_ssn.append (a2[1])
  status = cdbpy.dbsetchar (table, "FName", a2[2])
  if (status == -1) :
    print "%s %s:%s" % \
        ("\n\n***Error: dbsetchar table FName ", a2[2], cdbpy.dbgeterror())
    cdbpy.dbclose (table)
    sys.exit (1)
  new_lfname.append (a2[0] + a2[2])
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

table = cdbpy.dbopen ("flogdel3.db")
if (table == '') :
  print "Opening flogdel3.db the second time: %s" % (cdbpy.dbgeterror())
  sys.exit (1)

#
# Now, traverse through the table and insure that the data is consistent with
# what was actually entered.
#

t1 = 0.0
t1 = cdbpy.dbtime ()
counter = 0
cdbpy.dbcurrent (table, "fd3_ssnidx")
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

status = cdbpy.dbcurrent (table, "fd3_lnidx")
if (status == -1) :
  print "***Error: dbcurrent (%s, %s): " % (table, "fd3_lnidx")
  cdbpy.dbexit ()
  sys.exit (1)

for lname in new_lname :
  status = cdbpy.dbsearchexact (table, lname)
  if (status == 0) :
    print "***Warning: Could not find %s in LName index" % (lname)

status = cdbpy.dbcurrent (table, "fd3_ssnidx")
if (status == -1) :
  print "***Error: dbcurrent (%s, %s): " % (table, "fd3_ssnidx")
  cdbpy.dbexit ()
  sys.exit (1)

for ssn in new_ssn :
  status = cdbpy.dbsearchexact (table, ssn)
  if (status == 0) :
    print "***Warning: could not find %s in SSNumber index" % (ssn)

status = cdbpy.dbcurrent (table, "fd3_lfname")
if (status == -1) :
  print "***Error: dbcurrent (%s, %s): " % (table, "fd3_lfname")
  cdbpy.dbexit ()
  sys.exit (1)

for lfname in new_lfname :
  status = cdbpy.dbsearchexact (table, lfname)
  if (status == 0) :
    print "***Warning: could not find \"%s\" in LName, FName multi-index" % (lfname)

ssn_list = []
lname_list = []
lfname_list = []
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
    this_fname = cdbpy.dbshow (table, "FName")
    ssn_list.append (this_ssn)
    lname_list.append (this_lname)
    lfname_list.append (this_lname + this_fname)
    cdbpy.dbdel (table)
    cdbpy.dbnext (table)
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
  table = cdbpy.dbopen ("flogdel3.db")
  if (table == '') :
    print "\n\n***Error: couldn't open flogdel3.db second time: %s\n" % (cdbpy.dbgeterror())
    sys.exit (1)
  t2 = 0.0
  t2 = cdbpy.dbtime ()
  num_recs = cdbpy.dbnumrecs (table)
  print "Packed %s in %f seconds - %d records left in the table" % (table, t2-t1, num_recs)
  t1 = cdbpy.dbtime ()
  for ssn in ssn_list :
    if (cdbpy.dbsearchexact (table, ssn)) :
      print "***Warning: Could search for and retrieve deleted SSN: %s" % (ssn)
  cdbpy.dbcurrent (table, "fd3_lnidx")
  for lname in lname_list :
    if (cdbpy.dbsearchexact (table, lname)) :
      print "Found %s in lname index and shouldn't have\n" % (lname)
      sys.stdout.flush()
  cdbpy.dbcurrent (table, "fd3_lfname")
  for lfname in lfname_list :
    if (cdbpy.dbsearchexact (table, lfname)) :
      print "Found %s in last name, first name index and shouldn't have\n" % (lfname)
  t2 = cdbpy.dbtime ()
  numItems = len (ssn_list)
  print "Searched %d in %f seconds.\n" % (numItems, t2-t1)

print "There were %d records left after all in %s\n" % (cdbpy.dbnumrecs (table), table)

cdbpy.dbexit ()
sys.exit (0)

cdbpy.dbclose (table)

sys.exit (0)
