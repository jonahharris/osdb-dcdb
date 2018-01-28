#!/bin/sh
# The next line will execute tclsh on this script: \
  exec $TCLSH "$0" "$@"

#
# This test script is designed to exhaustively test table deletion and packing.
# It populates a table based on the items in testAdd.txt. Then, it makes
# 4 passes through the table, deleting $delNumber records, packing the
# table, closing it, opening it and searching for the deleted items.  It
# issues a warning error if it finds any of the deleted items in any of the
# passes.  Finally, it closes the table and exits.
#
# This verifies that once a table has been packed, the data within the
# table is still consistent.
#

#
# Set this to anything you want...if it is too high, it will have no bearing on
# how many items get deleted.  If this is greater than 1/4 of the value of keys
# in flogdel3.sh, then it will not effect what gets deleted.
#

if [ catch { source ./flogdel3_inc.tcl } result ] {
  puts stderr "Could not source ./flogdel3_inc.tcl: $result"
  exit 1
}
file delete -force ./flogdel3_inc.tcl

set SLE [info sharedlibextension]
if [ catch { load ../../lib/libcdbtcl$SLE } result ] {
  puts stderr "flogdel3.tcl: could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

if [ catch { set fdel3 [ open flogdel3.df w 0644 ] } result ] {
  puts stderr "Could not open flogdel3.df: $result"
  exit 1
}
puts $fdel3 {//flogdel3.df}
puts $fdel3 {create table "flogdel3.db"}
puts $fdel3 {  info "Table for flogdel3.tcl"}
puts $fdel3 "{"
puts $fdel3 {  "LName" char (65);}
puts $fdel3 {  "SSNumber" char (9);}
puts $fdel3 {  "FName" char (35);}
puts $fdel3 {  "StreetAddress" char (80);}
puts $fdel3 {  "City" char (50);}
puts $fdel3 {  "State" char (2);}
puts $fdel3 {  "Zip" char (10);}
puts $fdel3 {  "AnnualSalary" number (10:2);}
puts $fdel3 {  "HomeOwner" logical;}
puts $fdel3 {  "DateApplied" date;}
puts $fdel3 {  "LastUpdated" time;}
puts $fdel3 "\} indexed \{"
puts $fdel3 {  idx "fd3_ssnidx" 256:case:unique "SSNumber";}
puts $fdel3 {  idx "fd3_lnidx" 512:case:unique "LName";}
puts $fdel3 {  midx "fd3_lfname" 512:case "LName","FName";}
puts $fdel3 "\};"
puts $fdel3 ""
close $fdel3


set clock1 [ dbtime ]

set status [ file exists flogdel3.db ]
if { $status } {
  file delete -force flogdel3.db
  file delete -force flogdel3.db.LCK
  file delete -force fd3_ssnidx.inx
  file delete -force fd3_ssnidx.idx
  file delete -force fd3_lnidx.inx
  file delete -force fd3_lnidx.idx
  file delete -force fd3_lfname.inx
  file delete -force fd3_lfname.idx
}

if [ catch { dbcreate flogdel3.df } result ] {
  puts stderr "Creating table/indexes from flogdel3.df: $result"
  exit 1
}

if [ catch { set table [ dbopen flogdel3.db ] } result ] {
  puts stderr "Opening flogdel3.db: $result"
  dbexit
  exit 1
}
if {$table == ""} {
  puts stderr "Opening flogdel3.db: opened by another process."
  exit 1
}

if [ catch { set file [ open testAdd.txt ] } result ] {
  puts stderr "Opening file testAdd.txt: $result"
  dbexit
  exit 1
}

if [ catch { set lineList [ split [ read $file ] "\n" ] } result ] {
  puts stderr "Could not get the contents of testAdd.txt: $result"
  dbexit
  exit 1
}
close $file
file delete -force flogdel3.df

set new_lname [ list ]
set new_ssn [ list  ]
set new_lfname [ list ]

set counter 0
foreach line $lineList {
  incr counter
  set output [ expr $counter%1000 ]
  if { $output == 0 } {
    puts -nonewline "."
    flush stdout
  }
  if { $line != "" } {
    if [ catch { set itemList [ split $line ":" ] } ] {
      puts "Could not split line: \"$line\""
      dbexit
      exit 1
    }
    # LName
    set LName [ lindex $itemList 0 ]
    if { $LName == "" } {
      puts "***Warning: LName blank in line $counter"
      continue
    }
    if [ catch { dbsetchar $table LName $LName } result ] {
      puts "dbsetchar for LName: $result"
      dbexit
      exit 1
    }
    set SSNumber [ lindex $itemList 1 ]
    if { $SSNumber == "" } {
      puts "***Warning: SSNumber blank in line $counter"
      continue
    }
    if [ catch { dbsetchar $table SSNumber $SSNumber } result ] {
      puts "dbsetchar for SSNumber: $result"
      dbexit
      exit 1
    }
    set FName [ lindex $itemList 2 ]
    if { $FName == "" } {
      puts "***Warning: FName blank in line $counter"
      continue
    }
    if [ catch { dbsetchar $table FName $FName } result ] {
      puts "dbsetchar for FName: $result"
      dbexit
      exit 1
    }
    set StreetAddress [ lindex $itemList 3 ]
    if { $StreetAddress == "" } {
      puts "***Warning: StreetAddress blank in line $counter"
      continue
    }
    if [ catch { dbsetchar $table StreetAddress $StreetAddress } result ] {
      puts "dbsetchar for StreetAddress: $result"
      dbexit
      exit 1
    }
    set City [ lindex $itemList 4 ]
    if { $City == "" } {
      puts "***Warning: City is blank in line $counter"
      continue
    }
    if [ catch { dbsetchar $table City $City } result ] {
      puts "dbsetchar for City: $result"
      dbexit
      exit 1
    }
    set State [ lindex $itemList 5 ]
    if { $State == "" } {
      puts "***Warning: State is blank in line $counter"
      continue
    }
    if [ catch { dbsetchar $table State $State } result ] {
      puts "dbsetchar for State: $result"
      dbexit
      exit 1
    }
    set Zip [ lindex $itemList 6 ]
    if { $Zip == "" } {
      puts "***Warning: Zip is blank in line $counter"
      continue
    }
    if [ catch { dbsetchar $table Zip $Zip } result ] {
      puts "dbsetchar for Zip: $result"
      dbexit
      exit 1
    }
    set AnnualSalary [ lindex $itemList 7 ]
    if { $AnnualSalary == "" } {
      puts "***Warning: AnnualSalary is blank in line $counter"
      continue
    }
    if [ catch { dbsetnum $table AnnualSalary $AnnualSalary } result ] {
      puts "dbsetchar for AnnualSalary to $AnnualSalary: $result"
      dbexit
      exit 1
    }
    set HomeOwner [ lindex $itemList 8 ]
    if { $HomeOwner == "" } {
      puts "***Warning: HomeOwner is blank in line $counter"
      continue
    }
    if [ catch { dbsetlog $table HomeOwner $HomeOwner } result ] {
      puts "dbsetchar for HomeOwner: $result"
      dbexit
      exit 1
    }
    #
    # Just set DateApplied to the current information.
    #
    if [ catch { dbsetdate $table DateApplied 0 } result ] {
      puts "dbsetchar for DateApplied: $result"
      dbexit
      exit 1
    }
    #
    # We will just set the LastUpdated field to the current time
    #
    if [ catch { dbsettime $table LastUpdated 0 } result ] {
      puts "dbsettime for LastUpdated: $result"
      dbexit
      exit 1
    }
    #
    # Add them
    #
    if [ catch { dbadd $table } result ] {
      if { $result == "adding record: unique index constraint violated" } {
	puts ""
	puts "***Warning: Duplicate SSN at line $counter"
	lappend dupList $SSNumber
	continue
      }
      puts "Calling dbadd: $result"
      dbexit
      exit 1
    }
    lappend new_lname $LName
    lappend new_ssn $SSNumber
    set this_lfname [ format "%s%s" $LName $FName ]
    lappend new_lfname $this_lfname
  }
}

dbclose $table

if [ catch { set table [ dbopen flogdel3.db ] } result ] {
  puts stderr "Opening flogdel3.db: $result"
  exit 1
}
if {$table == ""} {
  puts stderr "Opening flogdel3.db: opened by another process."
  exit 1
}

set clock2 [ dbtime ]
set nowTime [ expr $clock2-$clock1 ]
set thisTime [ format "%6.4f" $nowTime ]

set num_table [ dbnumrecs $table ]
puts ""
puts "Finished adding to $table: $num_table records in $thisTime seconds"
puts ""

set status [ dbiseof $table ]
if { $status == 1 } {
  puts stderr "There are no records in $table!"
  dbexit
  exit 1
}
#

# OK, verify that the indexed data is all there.
#

set clock1 [ dbtime ]

if [ catch { dbcurrent $table "fd3_lnidx" } result ] {
  puts "***Error: dbcurrent $table fd3_lnidx failed: $result"
  dbexit
  exit 1
}
foreach lname $new_lname {
  set status [ dbsearchexact $table $lname ]
  if { ! $status } {
    puts "***Warning: could not find $lname in LName index"
  }
}
unset new_lname
if [ catch { dbcurrent $table "fd3_ssnidx" } result ] {
  puts "***Error: dbcurrent $table fd3_ssnidx failed: $result"
  dbexit
  exit 1
}
foreach ssn $new_ssn {
  set status [ dbsearchexact $table $ssn ]
  if { ! $status } {
    puts "***Warning: could not find $ssn in SSNumber index"
  }
}
unset new_ssn
if [ catch { dbcurrent $table "fd3_lfname" } result ] {
  puts "***Error: dbcurrent $table fd3_lfname failed: $result"
  dbexit
  exit 1
}
foreach lfname $new_lfname {
  set status [ dbsearchexact $table $lfname ]
  if { ! $status } {
    puts "***Warning: Could not find $lfname in LName,FName multi-index"
  }
}
unset new_lfname

set clock2 [ dbtime ]
set nowTime [ expr $clock2-$clock1 ]
set thisTime [ format "%6.4f" $nowTime ]

puts "Finished verifying all indexes in $thisTime seconds"

set ssn_list [ list ]
set lname_list [ list ]
set lfname_list [ list ]

#
# Iterate through the table 5 times, deleting a group of records as we go.
#
set pass 0
for { set i 0 } { $i < 4 } { incr i 1 } {
  incr pass 1
  set counter 0
  set step [ expr 5 - $i ]
  set clock1 [ dbtime ]
  puts ""
  puts "Pass $pass"
  dbhead $table
  for { set j 0 } { $j < $delNumber } { incr j 1 } {
    for { set k 0 } { $k < $step } {incr k 1} {
      dbnext $table
      set status [ dbiseof $table ]
      if { $status } {
	break
      }
    }
    set status [ dbiseof $table ]
    if { $status } {
      break
    }
    dbretrieve $table
    set this_lname [ dbshow $table LName ]
    set this_ssn [ dbshow $table SSNumber ]
    set this_fname [ dbshow $table FName ]
    set this_lfname [ format "%s%s" $this_lname $this_fname ]
    lappend ssn_list $this_ssn 
    lappend lname_list $this_lname
    lappend lfname_list $this_lfname
    dbdel $table
    incr counter 1
  }
  set clock2 [ dbtime ]
  set nowTime [ expr $clock2-$clock1 ]
  set thisTime [ format "%6.4f" $nowTime ]

  puts "$counter items deleted from $table in $thisTime seconds"
  puts "Packing $table:"

  set clock1 [ dbtime ]
  if [ catch { set status [ dbpack $table ] } result ] {
    puts stderr "packing $table: $result"
    dbexit
    exit 1
  }
  dbclose $table
  if [ catch {set table [ dbopen flogdel3.db ]} result ] {
    puts "Could not open flogdel3.db: $result"
    dbexit
    exit 1
  }
  if {$table == ""} {
    puts stderr "Opening flogdel3.db: opened by another process."
    exit 1
  }
  set clock2 [ dbtime ]
  set nowTime [ expr $clock2-$clock1 ]
  set thisTime [ format "%6.4f" $nowTime ]
  set num_recs [ dbnumrecs $table ]
  puts "Pack time was $thisTime seconds - $num_recs records left in $table"
  set clock1 [ dbtime ]
  dbcurrent $table fd3_ssnidx
  foreach ssn $ssn_list {
    set status [ dbsearchexact $table $ssn ]
    if { $status } {
      puts "***Warning: Could search for and retrieve deleted SSN: $ssn"
    }
  }
  dbcurrent $table fd3_lnidx
  foreach lname $lname_list {
    set status [ dbsearchexact $table $lname ]
    if { $status } {
      puts "***Warning: Could search and retrieve deleted LName: $lname"
    }
  }
  foreach lfname $lfname_list {
    set status [ dbsearchexact $table $lfname ]
    if { $status } {
      puts "***Warning: could search deleted LName/FName (probably not a problem): $lfname"
    }
  }
  set clock2 [ dbtime ]
  set nowTime [ expr $clock2-$clock1 ]
  set thisTime [ format "%6.4f" $nowTime ]
  set numitems [ llength $ssn_list ]
  puts "Searched $numitems in $thisTime seconds"
}

puts ""
puts "Finished successfully."
dbexit
exit 0
