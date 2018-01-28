#!/bin/sh
# Before you run this, set TCLSH to your version of tclsh (e.g., tclsh8.4).
# The next line will execute tclsh on this script: \
exec $TCLSH "$0" "$@"

set SLE [info sharedlibextension]
if [ catch { load ../../lib/libcdbtcl$SLE } result ] {
  puts stderr "Could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

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

for { set i 0 } { $i < $numItems } { incr i 1 } {
  set LName [ dbtestupperstring 0 64 ]
  set SSNumber [ dbtestnumber 9 0 ]
  set FName [ dbtestupperstring 0 34 ]
  set StreetAddress [ dbteststring 75 0 ]
  set City [ dbteststring 45 0 ]
  set State NM
  set Zip [ dbtestnumber 10 0 ]
  set AnnualSalary [ dbtestnumber 7 0 ]
  set HomeOwner Y
  set DateApplied [ dbtestnumber 8 0 ]
  set LastUpdated [ dbteststring 26 0 ]

  set outstr [ format "%s:%s:%s:%s:%s:%s:%s:%10.2f:%s:%s:%s" $LName $SSNumber $FName $StreetAddress $City $State $Zip $AnnualSalary $HomeOwner $DateApplied $LastUpdated ]
  puts $fp $outstr
}

close $fp
exit 0
