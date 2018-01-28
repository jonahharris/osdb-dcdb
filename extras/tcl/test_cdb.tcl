#!/bin/sh
# Use the shell... \
  exec tclsh "$0" "$@"

if [ catch { load ../../lib/libcdbtcl.so } result ] {
  puts "Couldn't load ../../lib/libcdbtcl.so: $result"
  return 1
}

#
# Test the support scripting in cdb.tcl.  Works on Mandrake 8.2 - your mileage
# may vary.
#
#source cdb.tcl
lappend auto_path /home/dfmay/devl/dcdb-1.2/extras/tcl
package require cdb 1.1

#
# Test the pseudo-random number generator.
#
puts ""
puts "*** Testing pseudo-random number generator."
puts ""
set one_rand [ cdb::random ]
cdb::randomInit 20030801
set next_rand [ cdb::random ]

puts "First random number: $one_rand, next: $next_rand"

set one_rand [ cdb::randomRange 100 ]
set next_rand [ cdb::randomRange 1000 ]

puts "First random number (range 100): $one_rand, next (range 1000): $next_rand"

#
# Test the isValidTable script.
#
puts ""
puts "*** Testing isValidTable script."
puts ""

catch { eval exec cp ../scripts/flogadd2_1.db . } 
catch { eval exec cp ../scripts/flogadd2_2.db . } 
catch { eval exec cp ../scripts/f2_1_lfmidx.idx . } 
catch { eval exec cp ../scripts/f2_1_lfmidx.inx . }
catch { eval exec cp ../scripts/f2_1_ssnidx.inx . }
catch { eval exec cp ../scripts/f2_1_ssnidx.idx . }
catch { eval exec cp ../scripts/f2_2_ssnidx.idx . }
catch { eval exec cp ../scripts/f2_2_ssnidx.inx . }

set table1 flogadd2_1.db
set table2 flogadd2_2.db

set valid [ cdb::isValidTable $table1 ]
if { ! $valid } {
  puts "$table1 is not valid or not open."
} else {
  puts "$table1 is a valid table that is open."
}

if [ catch { set tbl1 [ dbopen $table1 ] } result ] {
  puts "Could not open $table1: $result"
  dbexit
  return 1
}
set valid [ cdb::isValidTable $table1 ]
if { ! $valid } {
  puts "$table1 is not valid or not open."
} else {
  puts "Now, $table1 is a valid table that is open."
}

if [ catch { set tbl2 [ dbopen $table2 ] } result ] {
  puts "Could not open $table2: $result"
  dbexit
  return 1
}

#
# Test the tableListing script.
#
puts ""
puts "*** Testing tableListing script."
puts ""
set status [ cdb::listTable $tbl1 ]
set len [ llength $cdb::tableListing ]
if { $status } {
  puts -nonewline "***Error getting table information from $tbl1: "
  set outstr [ lindex $cdb::tableListing 0 ]
  puts $outstr
} else {
  for { set i 0 } { $i < $len } { incr i 1 } {
    set outstr [ lindex $cdb::tableListing $i ]
    puts $outstr
  }
}
puts ""
set status [ cdb::listTable $tbl2 ]
set len [ llength $cdb::tableListing ]
if { $status } {
  puts -nonewline "***Error getting table information from $tbl2: "
  set outstr [ lindex $cdb::tableListing 0 ]
  puts $outstr
} else {
  for { set i 0 } { $i < $len } { incr i 1 } {
    set outstr [ lindex $cdb::tableListing $i ]
    puts $outstr
  }
}

#
# Test listTableArray
#
puts ""
puts "*** Testing listTableArray."
puts ""
set status [ cdb::listTableArray $tbl1 ]
if { $status } {
  puts -nonewline "***Error: populating tableArray from $tbl1: $cdb::tableArray(error)"
} else {
  set outstr [ format "There are %d fields in $tbl1"  $cdb::tableArray(numfields) ]
  puts $outstr
  puts ""
  for { set i 0 } { $i < $cdb::tableArray(numfields) } { incr i 1 } {
    set outstr [ format "Field %s, type %s, len %d, declen %d, indexed %s" \
      $cdb::tableArray($i,name) \
      $cdb::tableArray($i,fieldtype) \
      $cdb::tableArray($i,fieldlen) \
      $cdb::tableArray($i,fielddec) \
      $cdb::tableArray($i,indexed) ]
    puts $outstr
  }
}
puts ""
set status [ cdb::listTableArray $tbl2 ]
if { $status } {
  puts -nonewline "***Error: populating tableArray from $tbl2: $cdb::tableArray(error)"
} else {
  set outstr [ format "There are %d fields in $tbl2"  $cdb::tableArray(numfields) ]
  puts $outstr
  puts ""
  for { set i 0 } { $i < $cdb::tableArray(numfields) } { incr i 1 } {
    set outstr [ format "Field %s, type %s, len %d, declen %d, indexed %s" \
      $cdb::tableArray($i,name) \
      $cdb::tableArray($i,fieldtype) \
      $cdb::tableArray($i,fieldlen) \
      $cdb::tableArray($i,fielddec) \
      $cdb::tableArray($i,indexed) ]
    puts $outstr
  }
}
dbexit

#
# Test the fileModeString script.
#
puts ""
puts "*** Testing fileModeString script."
puts ""
#set fileList [ list /usr/bin/suidperl /usr/bin/chsh /usr/bin/procmail \
#  /usr/bin/at /usr/bin/sudo /usr/bin/lppasswd /tmp ]
set fileList [ glob -directory /var/run * ]
set fileListLen [ llength $fileList ]
for { set i 0 } { $i < $fileListLen } { incr i 1 } {
  set file [ lindex $fileList $i ]
  set status [ cdb::fileModeString $file ]
  if { $status } {
    puts "$cdb::fileMode"
    return -1
  } else {
    puts "Mode of $file: $cdb::fileMode"
  }
}

file delete -force flogadd2_1.db
file delete -force flogadd2_2.db
file delete -force f2_1_lfmidx.idx
file delete -force f2_1_lfmidx.inx
file delete -force f2_1_ssnidx.inx
file delete -force f2_1_ssnidx.idx
file delete -force f2_2_ssnidx.idx
file delete -force f2_2_ssnidx.inx

puts ""
puts "Done."

return 0
