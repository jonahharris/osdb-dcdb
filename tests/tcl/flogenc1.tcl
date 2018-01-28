#!/bin/sh
# The next line will execute tclsh on this script: \
  exec $TCLSH "$0" "$@"

#
# First, load the cdbtcl.so file.  Note: this will fail if you have not done
#
# make cdbtcl.so
#
# in the main dcdb src directory.
#
set SLE [info sharedlibextension]
if [ catch { load ../../lib/libcdbtcl$SLE } result ] {
  puts "flogenc1.tcl: could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

set clock1 [ dbtime ]

puts "Parsing testAdd.txt"
if [ catch { set file [ open testAdd.txt ] } result ] {
  puts "Opening file testAdd.txt:"
  puts $result
  dbexit
  exit 1
}

if [ catch { set lineList [ split [ read $file ] "\n" ] } ] {
  puts "Could not get the contents of testAdd.txt:"
  dbexit
  exit 1
}
close $file

set passwd "to be or not to be"
set size 384

if [ catch { set fp [ open encrypted.txt w 0644 ] } result ] {
  puts "Could not create encrypted.txt: $result"
  dbexit
  exit 1
}
#
# First, populate the table with data based on what testAdd.txt generated.
#
set counter 0
# Create an empty list to hold the original (unencrypted) data items
set itemList [ list ]
foreach line $lineList {
  incr counter
  set output [ expr $counter%1000 ]
  if { $output == 0 } {
    puts -nonewline "."
    flush stdout
  }
  if { $line != "" } {
    lappend itemList $line
    set encrypted [ dbencrypt $line $size $passwd ]
    puts $fp $encrypted
  }
}
close $fp

puts ""
puts "Parsing encrypted.txt"
if [ catch { set file [ open encrypted.txt ] } result ] {
  puts "Opening file encrypted.txt:"
  puts $result
  dbexit
  exit 1
}

if [ catch { set lineList [ split [ read $file ] "\n" ] } ] {
  puts "Could not get the contents of testAdd.txt:"
  dbexit
  exit 1
}

set num 0
set counter 0
foreach line $lineList {
  incr counter
  set output [ expr $counter%1000 ]
  if { $output == 0 } {
    puts -nonewline "."
    flush stdout
  }
  if { $line != "" } {
    set decrypted [ dbdecrypt $line $size $passwd ]
    set match [ lindex $itemList $num ]
    if { $decrypted != $match } {
      puts "Decrypted string \"$decrypted\" doesn't match \"$match\""
    }
  }
  incr num 1
}

set clock2 [ dbtime ]
set nowTime [ expr $clock2-$clock1 ]
set thisTime [ format "%6.4f" $nowTime ]

puts ""
set numItems [ llength $lineList ]
# I'm not sure why I have to do this for it to work???
incr numItems -1
puts "Finished encrypting and unencrypting $numItems in $thisTime seconds"

dbexit
exit 0

