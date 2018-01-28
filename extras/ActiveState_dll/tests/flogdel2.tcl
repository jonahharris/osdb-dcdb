#!/bin/sh
# The next line will execute tclsh on this script: \
  exec $TCLSH "$0" "$@"

#
# This test script is designed to exhaustively test table deletion and packing.
# It populates a table based on the items in testAdd.txt (this should be at
# least 25000 items).  Then, it makes 4 passes through the table, deleting
# 5000 records, packing the table, closing it, opening it and searching for the
# deleted items.  It issues a warning error if it finds any of the deleted items
# in any of the passes.  Finally, it closes the table and exits.
#
# This should insure that once a table has been packed, the data within the
# table is still consistent.
#

if [ catch { source ./flogdel2_inc.tcl } result ] {
  puts stderr "Could not load ./flogdel2_inc.tcl: $result"
  exit 1
}
file delete -force ./flogdel2_inc.tcl

set SLE [info sharedlibextension]
if [ catch { load cdbtcl$SLE } result ] {
  puts stderr "flogdel2.tcl: could not load cdbtcl$SLE: $result"
  exit 1
}

proc dbtime { } {
  set t [clock clicks -milliseconds]
  return [expr $t / 1000.00]
}

if [ catch { set fdel2 [ open flogdel2.df w 0644 ] } result ] {
  puts stderr "Could not open flogdel2.df: $result"
  exit 1
}
puts $fdel2 {//flogdel2.df}
puts $fdel2 {create table "flogdel2.db"}
puts $fdel2 {  info "Table for flogdel2.tcl"}
puts $fdel2 "{"
puts $fdel2 {  "LName" char (65);}
puts $fdel2 {  "SSNumber" char (9);}
puts $fdel2 {  "FName" char (35);}
puts $fdel2 {  "StreetAddress" char (80);}
puts $fdel2 {  "City" char (50);}
puts $fdel2 {  "State" char (2);}
puts $fdel2 {  "Zip" char (10);}
puts $fdel2 {  "AnnualSalary" number (10:2);}
puts $fdel2 {  "HomeOwner" logical;}
puts $fdel2 {  "DateApplied" date;}
puts $fdel2 {  "LastUpdated" time;}
puts $fdel2 "\} indexed \{"
puts $fdel2 {  idx "fd2_ssnidx" 256:case:unique "SSNumber";}
puts $fdel2 {  idx "fd2_lnidx" 512:case:unique "LName";}
puts $fdel2 "\};"
puts $fdel2 ""
close $fdel2


set clock1 [ dbtime ]

set status [ file exists flogdel2.db ]
if { $status } {
  file delete -force flogdel2.db
  file delete -force flogdel2.db.LCK
  file delete -force fd2_ssnidx.inx
  file delete -force fd2_ssnidx.idx
  file delete -force fd2_lnidx.inx
  file delete -force fd2_lnidx.idx
}

if [ catch { dbcreate flogdel2.df } result ] {
  puts stderr "Creating table/indexes from flogdel2.df: $result"
  exit 1
}

if [ catch { set table [ dbopen flogdel2.db ] } result ] {
  puts stderr "Opening flogdel2.db: $result"
  dbexit
  exit 1
}
if {$table == ""} {
  puts stderr "Opening flogdel2.db: opened by another process."
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
file delete -force flogdel2.df

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
  }
}

dbclose $table

if [ catch { set table [ dbopen flogdel2.db ] } result ] {
  puts stderr "Opening flogdel2.db: $result"
  exit 1
}
if {$table == ""} {
  puts stderr "Opening flogdel2.db: opened by another process."
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
set ssn_list [ list ]
set lname_list [ list ]

#
# Iterate through the table 5 times, deleting a group of records as we go.
#
set pass 0
for { set i 0 } { $i < 4 } { incr i 1 } {
  incr pass 1
  set counter 0
  set step [ expr 5 - $i ]
  set clock1 [ dbtime ]
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
    lappend ssn_list $this_ssn 
    lappend lname_list $this_lname
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
  if [ catch {set table [ dbopen flogdel2.db ]} result ] {
    puts "Could not open flogdel2.db: $result"
    dbexit
    exit 1
  }
  if {$table == ""} {
    puts stderr "Opening flogdel2.db: opened by another process."
    exit 1
  }
  set clock2 [ dbtime ]
  set nowTime [ expr $clock2-$clock1 ]
  set thisTime [ format "%6.4f" $nowTime ]
  set num_recs [ dbnumrecs $table ]
  puts "Pack time was $thisTime seconds - $num_recs records left in $table"
  set clock1 [ dbtime ]
  dbcurrent $table fd2_ssnidx
  foreach ssn $ssn_list {
    set status [ dbsearchexact $table $ssn ]
    if { $status } {
      puts "***Warning: Could search for and retrieve deleted SSN: $ssn"
    }
  }
  dbcurrent $table fd2_lnidx
  foreach lname $lname_list {
    set status [ dbsearchexact $table $lname ]
    if { $status } {
      puts "***Warning: Could search and retrieve deleted LName: $lname"
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
puts ""
dbexit
exit 0
