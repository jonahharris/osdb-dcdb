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
if [catch {load ../../lib/libcdbtcl$SLE} result] {
  puts stderr "flogadd1.tcl: could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

#
# Proc: check_dups SSNumber
#
# Checks to see if the Social Security number given by SSNumber is one of the
# duplicates in this set.  If so, it returns 1, otherwise 0.
#
proc check_dups { ssn } {
  global dupList
  set status [ llength $dupList ]
  if { ! $status } {
    return 0
  }
  foreach item $dupList {
    if { $item != "" } {
      if { $item == $ssn } {
	return 1
      }
    }
  }
  return 0
}

set clock1 [ dbtime ]

set fadd1 [ open flogadd1.df w 0644 ]
puts $fadd1 {// flogadd1.df}
puts $fadd1 {create table "flogadd1.db"}
puts $fadd1 {  info "Table for flogadd1.tcl"}
puts $fadd1 "{"
puts $fadd1 {  "LName" char (65);}
puts $fadd1 {  "SSNumber" char (9);}
puts $fadd1 {  "FName" char (35);}
puts $fadd1 {  "StreetAddress" char (80);}
puts $fadd1 {  "City" char (50);}
puts $fadd1 {  "State" char (2);}
puts $fadd1 {  "Zip" char (10);}
puts $fadd1 {  "AnnualSalary" number (10:2);}
puts $fadd1 {  "HomeOwner" logical;}
puts $fadd1 {  "DateApplied" date;}
puts $fadd1 {  "LastUpdated" time;}
puts $fadd1 "\} indexed \{"
puts $fadd1 {  idx "f1_ssnidx" 256:case:unique "SSNumber";}
puts $fadd1 {  idx "f1_lnidx" 512:case:dup "LName";}
puts $fadd1 "\};"
puts $fadd1 ""
close $fadd1

#
# Delete any tables/indexes from a previous iteration.
#
set status [ file exists flogadd1.db ]
if { $status } {
  file delete -force flogadd1.db
  file delete -force flogadd1.db.LCK
  file delete -force f1_ssnidx.inx
  file delete -force f1_ssnidx.idx
  file delete -force f1_lnidx.inx
  file delete -force f1_lnidx.idx
}

#
# Create the table and indexes.
#
if [ catch { dbcreate flogadd1.df } result ] {
  puts stderr "Creating table/indexes from flogadd1.df:"
  puts stderr "$result"
  exit 1
}

if [ catch { set table [ dbopen flogadd1.db ] } result ] {
  puts stderr "Opening flogadd1.db: "
  puts stderr $result
  exit 1
}
if {$table == ""} {
  puts stderr ("Couldn't open flogadd1.db: opened by another process?"
  exit 1
}
file delete -force flogadd1.df

puts "Parsing testAdd.txt"
if [ catch { set file [ open testAdd.txt ] } result ] {
  puts stderr "Opening file testAdd.txt:"
  puts stderr $result
  dbexit
  exit 1
}

if [ catch { set lineList [ split [ read $file ] "\n" ] } ] {
  puts stderr "Could not get the contents of testAdd.txt:"
  dbexit
  exit 1
}
close $file

#
# First, populate the table with data based on what testAdd.txt generated.
#
set counter 0
# Create an empty list to hold any duplicate SSNs.
set dupList [ list ]
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

if [ catch { set table [ dbopen flogadd1.db ] } result ] {
  puts stderr "Opening flogadd1.db the second time: "
  puts stderr $result
  exit 1
}
if {$table == ""} {
  puts stderr ("Couldn't open flogadd1.db: opened by another process?"
  exit 1
}

set clock2 [ dbtime ]
set nowTime [ expr $clock2-$clock1 ]
set thisTime [ format "%6.4f" $nowTime ]

set num_table [ dbnumrecs $table ]
puts ""
puts "Finished adding to $table: $num_table records added in $thisTime seconds"


set clock1 [ dbtime ]

#
# Now, traverse through the table and make sure the data is consistent with
# what was entered.
#
set counter 0
dbcurrent $table f1_ssnidx
set status [dbiseof $table]
if { $status == 1 } {
  puts stderr "There are no records in $table!"
  dbexit
  exit 1
}

foreach line $lineList {
  incr counter
  if { $line != "" } {
    if [ catch { set itemList [ split $line ":" ] } ] {
      puts "Could not split line: \"$line\""
      dbexit
      exit 1
    }
    set LName [ lindex $itemList 0 ]
    set SSNumber [ lindex $itemList 1]
    set FName [ lindex $itemList 2]
    set StreetAddress [ lindex $itemList 3]
    set City [ lindex $itemList 4]
    set State [ lindex $itemList 5]
    set Zip [ lindex $itemList 6]
    set AnnualSalary [ lindex $itemList 7]
    set HomeOwner [ lindex $itemList 8]

    set status [ dbsearchexact $table $SSNumber ]
    if { $status == 0 } {
      puts stderr "***Warning: couldn't find $SSNumber in the database: line $counter"
      continue
    }
    # Found it, get it and move on.
    if [ catch { dbretrieve $table } result ] {
      puts stderr "Could not retrieve from $table: $result"
      dbexit
      exit 1
    }
    set header 0
    set thisLName [ dbshow $table LName ]
    if { $LName != $thisLName } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	continue
      }
      if { ! $header } {
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " LName"
    }
    set thisSSNumber [ dbshow $table SSNumber ]
    if { $SSNumber != $thisSSNumber } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	continue
      }
      if { ! $header } {
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " SSNumber"
    }
    set thisFName [ dbshow $table FName ]
    if { $FName != $thisFName } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	continue
      }
      if { ! $header } {
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " FName"
    }
    set thisStreetAddress [ dbshow $table StreetAddress ]
    if { $StreetAddress != $thisStreetAddress } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	if { ! $header } {
	  continue
	}
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " StreetAddress"
    }
    set thisCity [ dbshow $table City ]
    if { $City != $thisCity } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	if { ! $header } {
	  continue
	}
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " City"
    }
    set thisState [ dbshow $table State ]
    if { $State != $thisState } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	continue
      }
      if { ! $header } {
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " State"
    }
    set thisZip [ dbshow $table Zip ]
    if { $Zip != $thisZip } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	continue
      }
      if { ! $header } {
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " Zip"
    }
    set thisAnnualSalary [ dbshow $table AnnualSalary ]
    if { $AnnualSalary != $thisAnnualSalary } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	continue
      }
      if { ! $header } {
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " AnnualSalary"
    }
    set thisHomeOwner [ dbshow $table HomeOwner ]
    if { $HomeOwner != $thisHomeOwner } {
      set status [ check_dups $SSNumber ]
      if { $status } {
	continue
      }
      if { ! $header } {
	puts -nonewline "Record out of sync:"
	set header 1
      }
      puts -nonewline " HomeOwner"
    }
    if { $header } {
      puts " at line $counter"
    }
  }
}

dbexit

set clock2 [ dbtime ]
set nowTime [ expr $clock2-$clock1 ]
set thisTime [ format "%6.4f" $nowTime ]

puts ""
puts "Verified the table data in $thisTime seconds"
exit 0

