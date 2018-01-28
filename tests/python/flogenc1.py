#!/usr/bin/python2.5

# Load the python module.
import sys
import os
sys.path[0] = ''
os.chdir ("../../lib")
import cdbpy
os.chdir ("../tests/python")

# Lock and load.
t1 = 0.0
t1 = cdbpy.dbtime ()

s = open ("testAdd.txt").read ()
a1 = []
a1 = s.split ()

this_passwd = "to be or not to be"
size = 384

enc1 = open ("encrypted.txt", "w", 0644)
counter = 0
itemList = []
for line in a1 :
  counter += 1
  if (counter % 1000 == 0) :
    sys.stdout.write (".")
    sys.stdout.flush()
  if (line != '') :
    itemList.append (line)
    encrypted = cdbpy.dbencrypt (line, size, this_passwd)
    file.write (enc1, encrypted)
    file.write (enc1, "\n")

print "\n"

file.close (enc1)

print "\nParsing encrypted.txt\n"

s = open ("encrypted.txt").read ()
a2 = []
a2 = s.split ()

num = 0
counter = 0
for line in a2 :
  counter += 1
  if (counter % 1000 == 0) :
    sys.stdout.write (".")
    sys.stdout.flush ()
  if (line != '') :
    this_decrypted = cdbpy.dbdecrypt (line, size, this_passwd)
    if (this_decrypted != a1[num]) :
      print "Decrypted string %s doesn't match %s\n" % (this_decrypted, a1[num])
    num += 1

t2 = 0.0
t2 = cdbpy.dbtime ()
numItems = len (a1)

print "\nFinished encrypting and unencrypting %d in %f seconds\n" % (numItems, t2-t1)

cdbpy.dbexit ()
sys.exit (0)

