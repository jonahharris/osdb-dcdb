#!/bin/sh
# Before you run this, set TCLSH to your version of tclsh (e.g., tclsh8.4).
# The next line will execute tclsh on this script: \
exec $TCLSH "$0" "$@"

set SLE [info sharedlibextension]
if [ catch { load ../../lib/libcdbtcl$SLE } result ] {
  puts stderr "Could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

if { $argc != 3 } {
  puts "Usage: mkstr.tcl 0 <size> number"
  puts "or     mkstr.tcl <size> 0 number"
  puts "If the second argument is 0, the strings generated will be of"
  puts "constant size <size>.  If the first argument is 0, they will"
  puts "vary in size, upt to <size> in length.  The third argument is"
  puts "the number of strings to generate."
  exit 1
}
set num1 [ lindex $argv 0 ]
set num2 [ lindex $argv 1 ]
set numItems [ lindex $argv 2 ]
if { $num1 == 0 && $num2 == 0 } {
  puts "Both arguments cannot be 0"
  exit 1
}
if { $num1 != 0 && $num2 != 0 } {
  puts "One of the arguments must be 0"
  exit 1
}

if [ catch {set fp [ open testAdd.txt w 0644 ] } result ] {
  puts stderr "Couldn't create testAdd.txt: $result"
  exit 1
}

for { set i 0 } { $i < $numItems } { incr i 1 } {
  set output [ dbteststring $num1 $num2 ]
  puts $fp $output
}

close $fp
exit 0
