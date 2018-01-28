#!/bin/sh
# The next line will execute tclsh on this script: \
exec $TCLSH "$0" "$@"

#
# First, load the libcdbtcl.so file.
#
set SLE [info sharedlibextension]
if [catch {load ../../lib/libcdbtcl$SLE} result] {
  puts stderr "flogadd1.tcl: could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

if {$argc != 1} {
  puts "Usage: flognum2-gen num"
	puts "  where num is the number of items to generate."
	exit 1
} else {
	set numiterations [lindex $argv 0]
}

set scale [dbtestnumber 2 0]
set scale [expr $scale % 20]
set numsize1 [dbtestnumber 2 0]
set numsize [expr $numsize1 + $scale]
if [catch {set fp [open testAdd.txt w 0644]} result] {
	puts "Could not create testAdd.txt: $result"
	exit 1
}
for {set i 0} {$i < $numiterations} {incr i 1} {
	set num1first [dbtestnumber $numsize1 0]
  set num1second [dbtestnumber $scale 0]
  set num1 [format "%s.%s" $num1first $num1second]
  set num2first [dbtestnumber 0 $numsize]
  set num2second [dbtestnumber 0 $scale]
  set num2 [format "%s.%s" $num2first $num2second]
  set op [dbtestnumber 4 0]
  set op [expr $op %  4]
  if {$op == 0} {
		set ops "+"
	}
	if {$op == 1} {
		set ops "-"
	}
	if {$op == 2} {
		set ops "x"
	}
	if {$op == 3} {
		set ops "/"
	}
	if {$op < 0 || $op > 3} {
		set ops "+"
	}
	set outstr [format "%d:%s:%s:%s" $scale $num1 $ops $num2]
  puts $fp $outstr
}
close $fp
exit 0
