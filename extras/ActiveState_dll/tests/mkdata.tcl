#!/bin/sh
# Before you run this, set TCLSH to your version of tclsh (e.g., tclsh8.4).
# The next line will execute tclsh on this script: \
exec $TCLSH "$0" "$@"

proc dbtime { } {
  set t [clock clicks -milliseconds]
  return [expr $t / 1000.00]
}

source test.tcl

if { $argc == 0 } {
  set numItems 5000
}
if { $argc == 1 } {
  set numItems [ lindex $argv 0 ]
}

if [ catch {set fp [ open testAdd.txt w 0644 ] } result ] {
  puts stderr "Couldn't create testAdd.txt: $result"
  exit 1
}

set clock1 [dbtime]

for { set i 0 } { $i < $numItems } { incr i 1 } {
  set status [expr $i % 1000]
  if {$status == 999} {
    puts -nonewline "."
    flush stdout
  }
  #rand_init [clock click -milliseconds]
  set LName [ testupperstring 0 64 ]
  set SSNumber [ testnumber 9 0 ]
  set FName [ testupperstring 0 34 ]
  set StreetAddress [ teststring 75 0 ]
  set City [ teststring 45 0 ]
  set State NM
  set Zip [ testnumber 10 0 ]
  set AnnualSalary [ testnumber 7 0 ]
  set HomeOwner Y
  set DateApplied [ testnumber 8 0 ]
  set LastUpdated [ teststring 26 0 ]

  set outstr [ format "%s:%s:%s:%s:%s:%s:%s:%10.2f:%s:%s:%s" $LName $SSNumber $FName $StreetAddress $City $State $Zip $AnnualSalary $HomeOwner $DateApplied $LastUpdated ]
  puts $fp $outstr
}

close $fp
set clock2 [dbtime]
set nowTime [expr $clock2-$clock1]
set thisTime [format "%6.3f" $nowTime]

puts ""
puts "Generated $numItems in $nowTime seconds."
puts ""
puts ""

exit 0
