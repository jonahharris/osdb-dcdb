#!/bin/sh
# The next line will execute tclsh on this script: \
exec $TCLSH "$0" "$@"

#
# First, load the libcdbtcl.so file.
#
set SLE [info sharedlibextension]
if [catch {load cdbtcl$SLE} result] {
  puts stderr "flogadd1.tcl: could not load dbtcl$SLE: $result"
  exit 1
}

source test.tcl

if {$argc == 1} {
  set numIterations [lindex $argv 0]
  if {$numIterations < 2} {
    set numIterations 2
  }
  if {$numIterations > 100} {
    set numIterations 100
  }
} else {
  puts "Usage: flognum1.tcl numIterations"
  exit 1
}

puts ""
puts "***Exercise dbnumadd..."
puts ""
for {set i 1} {$i <= $numIterations} {incr i 1} {
  puts -nonewline "Iteration $i "
  #rand_init [clock click -milliseconds]
  set val [testnumber 10 0]
  append val ".00"
  set counter 0
  while { $val > 0 } {
    incr counter 1
    set status [expr $counter % 10000]
    if { $status == 0 } {
      puts -nonewline "."
      flush stdout
    }
    set val1 $val
    set num [testnumber 5 0]
    set sub "-"
    append sub $num
    set val [bcnumadd $val $sub 2]
    set val1 [expr $val1 + $sub]
    set status [bcnumcompare $val $val1 2]
    if { $status != 0} {
      puts "$val and $val1 are not equal and they should be"
    } 
  }
  puts ""
  puts "counter = $counter"
}

puts ""
puts "***Exercise dbnumsub..."
puts ""
for {set i 1} {$i <= $numIterations} {incr i 1} {
  puts -nonewline "Iteration $i "
  set val [testnumber 10 0]
  append val ".00"
  set counter 0
  while { $val > 0 } {
    incr counter 1
    set status [expr $counter % 10000]
    if { $status == 0 } {
      puts -nonewline "."
      flush stdout
    }
    set val1 $val
    set num [testnumber 5 0]
    set val [bcnumsub $val $num 2]
    set val1 [expr $val1 - $num]
    set status [bcnumcompare $val $val1 2]
    if { $status != 0} {
      puts "$val and $val1 are not equal and they should be"
    } 
  }
  puts ""
  puts "counter = $counter"
}

puts ""
puts "***Exercise dbnumdivide..."
puts ""
for {set i 1} {$i <= $numIterations} {incr i 1} {
  puts -nonewline "Iteration $i "
  set val [testnumber 50 0]
  append val ".00"
  set counter 0
  while { $val > 2 } {
    incr counter 1
    set status [expr $counter % 100]
    if { $status == 0 } {
      puts -nonewline "."
      flush stdout
    }
    set num [testnumber 2 0]
    set val [bcnumdivide $val $num 2]
  }
  puts ""
  puts "counter = $counter, val = $val"
}

puts ""
puts "***Exercise dbnummultiply..."
puts ""
for {set i 1} {$i <= $numIterations} {incr i 1} {
  puts -nonewline "Iteration $i "
  set val [testnumber 2 0]
  set topval [testnumber 60 0]
  append val ".00"
  append topval ".00"
  set counter 0
  while { $val < $topval } {
    incr counter 1
    set status [expr $counter % 10]
    if { $status == 0 } {
      puts -nonewline "."
      flush stdout
    }
    set num [testnumber 2 0]
    set val [bcnummultiply $val $num 2]
  }
  puts ""
  puts "counter = $counter"
}

puts ""
puts "***Exercise dbnumraise..."
puts ""
for {set i 1} {$i <= $numIterations} {incr i 1} {
  puts -nonewline "Iteration $i "
  set val [expr $i+1]
  set topval [testnumber 60 0]
  append val ".00"
  append topval ".00"
  set counter 0
  while { $val < $topval } {
    incr counter 1
    set val [bcnumraise $val 2.00 2]
  }
  puts ""
  puts "counter=$counter"
}

#
# Uninitialize the bcnum stuff.
#
bcnumuninit
puts ""

exit 0
