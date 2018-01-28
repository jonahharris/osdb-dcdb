#!/usr/bin/python2.5

# Load the python module.
import sys
import os
sys.path[0] = ''
os.chdir ("../../lib")
import cdbpy
os.chdir ("../tests/python")

# First, get rid of the old stuff, if it exists.
if (os.access ("flogadd2_1.db", os.F_OK)) :
  os.remove ("flogadd2_1.db")
if (os.access ("flogadd2_1.db.LCK", os.F_OK)) :
  os.remove ("flogadd2_1.db.LCK")
if (os.access ("f2_1_ssnidx.inx", os.F_OK)) :
  os.remove ("f2_1_ssnidx.inx")
if (os.access ("f2_1_ssnidx.idx", os.F_OK)) :
  os.remove ("f2_1_ssnidx.idx")
if (os.access ("f2_1_lfmidx.inx", os.F_OK)) :
  os.remove ("f2_1_lfmidx.inx")
if (os.access ("f2_1_lfmidx.idx", os.F_OK)) :
  os.remove ("f2_1_lfmidx.idx")

if (os.access ("flogadd2_2.db", os.F_OK)) :
  os.remove ("flogadd2_2.db")
if (os.access ("flogadd2_2.db.LCK", os.F_OK)) :
  os.remove ("flogadd2_2.db.LCK")
if (os.access ("f2_2_ssnidx.inx", os.F_OK)) :
  os.remove ("f2_2_ssnidx.inx")
if (os.access ("f2_2_ssnidx.idx", os.F_OK)) :
  os.remove ("f2_2_ssnidx.idx")

fadd2 = open ("flogadd2.df", "w", 0644)
file.write (fadd2, '// flogadd2.df\n')
file.write (fadd2, 'create table "flogadd2_1.db"\n')
file.write (fadd2, '  info "Table for flogadd2.py"\n')
file.write (fadd2, '{\n')
file.write (fadd2, '  "LName" char (65);\n')
file.write (fadd2, '  "SSNumber" char (9);\n')
file.write (fadd2, '  "FName" char (35);\n')
file.write (fadd2, '  "NumberLicenses" number (2:0);\n')
file.write (fadd2, '} indexed {\n')
file.write (fadd2, '  idx "f2_1_ssnidx" 256:case:unique "SSNumber";\n')
file.write (fadd2, '  midx "f2_1_lfmidx" 512:case "LName", "FName";\n')
file.write (fadd2, '};\n\n')
file.write (fadd2, 'create table "flogadd2_2.db"\n')
file.write (fadd2, '  info "Table for flogadd2.py"\n')
file.write (fadd2, '{\n')
file.write (fadd2, '  "SSNumber" char (9);\n')
file.write (fadd2, '  "License" char (7);\n')
file.write (fadd2, '} indexed {\n')
file.write (fadd2, '  idx "f2_2_ssnidx" 256:case:dup "SSNumber";\n')
file.write (fadd2, '};\n\n')
file.close (fadd2)

status = cdbpy.dbcreate ("flogadd2.df")
if (status != 0) :
  print "%s %s" % ("\n\n***Error dbcreate flogadd2.df: ", cdbpy.dbgeterror ())
  sys.exit (1)

# Fire at will.
table = cdbpy.dbopen ("flogadd2_1.db")
if (table == '') :
  print "\n\n***Error: dbopen flogadd2_1.db: %s" % (cdbpy.dbgeterror())
  sys.exit (1)
tbl1 = cdbpy.dbopen ("flogadd2_2.db")
if (table == '') :
  print "\n\n***Error: dbopen flogadd2_2.db: %s" % (cdbpy.dbgeterror())
  sys.exit (1)

os.remove ("flogadd2.df")

counter = 0
modulus = 10
mod_inc = 1
ssnList = []

t1 = 0.0
t1 = cdbpy.dbtime ()

import random
random.seed (None)

execfile ("fa_inc.py")
while (counter < numLoop) :
  counter += 1
  if (counter % 1000 == 0) :
    sys.stdout.write ('.')
    sys.stdout.flush()
  LName = cdbpy.dbtestupperstring (0, 65)
  if (LName == '') :
    print "\n\n***Error: dbtestupperstring 0 65: %s\n" % \
        (cdbpy.dbgeterror ())
    cdbpy.dbexit ()
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "LName", LName)
  if (status == -1) :
    print "\n\n***Error: dbsetchar %s LName %s:%s" % \
        (table, LName, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  SSNumber = cdbpy.dbtestnumber (9, 0)
  if (SSNumber == '') :
    print "\n\n***Error: dbtestnumber 9 0: %s\n" % \
        (cdbpy.dbgeterror ())
    cdbpy.dbexit ()
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "SSNumber", SSNumber)
  if (status == -1) :
    print "\n\n***Error: dbsetchar %s SSNumber %s:%s" % \
        (table, SSNumber, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  FName = cdbpy.dbtestupperstring (0, 35)
  if (FName == '') :
    print "\n\n***Error: dbtestupperstring 0 35: %s\n" % \
        (cdbpy.dbgeterror ())
    cdbpy.dbexit ()
    sys.exit (1)
  status = cdbpy.dbsetchar (table, "FName", FName)
  if (status == -1) :
    print "\n\n***Error: dbsetchar %s FName %s:%s" % \
        (table, FName, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  RAND = random.randint (0, modulus)
  if (RAND == 0) : RAND = 10
  modulus += mod_inc
  if (modulus > 40) : mod_inc = -1
  if (modulus < 10) : mod_inc = 1
  status = cdbpy.dbsetnum (table, "NumberLicenses", RAND)
  if (status == -1) :
    print "\n\n***Error: dbsetnum %s NumberLicenses %s:%s" % \
        (table, RAND, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  status = cdbpy.dbadd (table)
  if (status == -1) :
    terr = cdbpy.dbgeterror()
    status = cmp (terr, "adding record: unique index constraint violated")
    if not status :
      counter -= 1
      continue
    print "\n\n***Error: dbadd %s:%s\n" % (table, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  newcount = 0
  while (newcount < RAND) :
    newcount += 1
    status = cdbpy.dbsetchar (tbl1, "SSNumber", SSNumber)
    if (status == -1) :
      print "\n\n***Error: dbsetchar %s SSNumber %s:%s" % \
          (tbl1, SSNumber, cdbpy.dbgeterror())
      cdbpy.dbexit ()
      sys.exit (1)
    license = cdbpy.dbtestnumstring (7, 0)
    if (license == '') :
      print "\n\n***Error: dbtestnumstring (7, 0): %s\n" % \
          (cdbpy.dbgeterror())
      cdbpy.dbexit()
      sys.exit (1)
    status = cdbpy.dbsetchar (tbl1, "License", license)
    if (status == -1) :
      print "\n\n***Error: dbsetchar %s License %s:%s" % \
          (tbl1, license, cdbpy.dbgeterror())
      cdbpy.dbexit ()
      sys.exit (1)
    status = cdbpy.dbadd (tbl1)
  ssnList.append (SSNumber)

cdbpy.dbclose (table)
cdbpy.dbclose (tbl1)

table = cdbpy.dbopen ("flogadd2_1.db")
if (table == '') :
  print "Opening flogadd2_1.db the second time: %s" % (cdbpy.dbgeterror())
  sys.exit (1)
tbl1 = cdbpy.dbopen ("flogadd2_2.db")
if (tbl1 == '') :
  print "Opening flogadd2_2.db the second time: %s" % (cdbpy.dbgeterror())
  cdbpy.dbclose (table)
  sys.exit (1)

t2 = 0.0
t2 = cdbpy.dbtime ()

num_table = cdbpy.dbnumrecs (table)
num_tbl1 = cdbpy.dbnumrecs (tbl1)

print "\nFinished adding...\n"
print "%d items in %s and %d items in %s\n" % \
    (num_table, table, num_tbl1, tbl1)
print "Done in %f seconds" % (t2-t1)

counter = 0
cdbpy.dbcurrent (table, "f2_1_ssnidx")
cdbpy.dbcurrent (tbl1, "f2_2_ssnidx")
status = cdbpy.dbiseof (table)
if (status == 1) :
  print "There are no records in %s\n" % (table)
  cdbpy.dbexit ()
  sys.exit (1)

t1 = 0.0
t1 = cdbpy.dbtime ()

for ssn in ssnList :
  counter += 1
  status = cdbpy.dbsearchexact (table, ssn)
  if (status == 0) :
    print "Could not retrieve from %s: %s\n" % (table, cdbpy.dbgeterror())
    continue
  status = cdbpy.dbretrieve (table)
  if (status == 0) : continue  # deleted record
  if (status == -1) :
    print "\n\n***Error: dbretrieve %s: %s\n" % \
        (table, cdbpy.dbgeterror())
    cdbpy.dbexit ()
    sys.exit (1)
  numlic = cdbpy.dbshow (table, "NumberLicenses")
  if (numlic == '') : continue
  numLicenses = int (numlic)
  status = cdbpy.dbsearchexact (tbl1, ssn)
  if (status == 0) :
    print "***Warning: Couldn't find %s in %s\n" % \
        (ssn, tbl1)
    continue
  i = 0
  while (i < numLicenses) :
    i += 1
    status = cdbpy.dbretrieve (tbl1)
    if (status == 0) : cdbpy.dbnextindex (tbl1)
    if (status == -1) :
      print "\n\n***Error: dbretrieve %s: %s\n" % \
          (tbl1, cdbpy.dbgeterror())
      cdbpy.dbexit ()
      sys.exit (1)
    this_ssn = cdbpy.dbshow (tbl1, "SSNumber")
    if (this_ssn == '') : continue
    if (this_ssn != ssn) :
      print "***Warning: ssn from %s (%s) != ssn from %s (%s)\n" % \
          (table, ssn, tbl1, this_ssn)
    cdbpy.dbnextindex (tbl1)

cdbpy.dbexit ()
if (os.access ("fa_inc.py", os.F_OK)) :
  os.remove ("fa_inc.py")

t2 = 0.0
t2 = cdbpy.dbtime ()

print "\n\nVerified the table data in %f seconds" % (t2-t1)
sys.exit (0)
