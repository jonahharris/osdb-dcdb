#!/bin/sh
# The next line will execute tclsh on this script: \
  exec tclsh "$0" "$@"

#
# Load cdbtcl.{so,dll}
#
set SLE [info sharedlibextension]
if [catch {load ../../lib/libcdbtcl$SLE} result] {
  puts "flogupd1.tcl: couldn't load ../../lib/libcdbtcl$SLE: $result"
  return 1
}

#
# Create the table.
#
if [catch {set fupd1 [open flogupd1.df w 0644]} result] {
  puts stderr "Could not open flogupd1.df to write: $result"
  exit 1
}
puts $fupd1 {//flogupd1.df}
puts $fupd1 {create table "flogupd1.db"}
puts $fupd1 {  info "Table for flogupd1.db"}
puts $fupd1 "{"
puts $fupd1 {  "LName" char (65);}
puts $fupd1 {  "SSNumber" char (9);}
puts $fupd1 {  "FName" char (35);}
puts $fupd1 {  "StreetAddress" char (75);}
puts $fupd1 {  "City" char (50);}
puts $fupd1 {  "State" char (2);}
puts $fupd1 {  "Zip" char (10);}
puts $fupd1 {  "AnnualSalary" number (10:2);}
puts $fupd1 {  "HomeOwner" logical;}
puts $fupd1 {  "DateApplied" date;}
puts $fupd1 {  "LastUpdated" time;}
puts $fupd1 "\} indexed \{"
puts $fupd1 {  idx "fu1_ssnidx" 256:case:unique "SSNumber";}
puts $fupd1 {  idx "fu1_lnidx" 512:case:unique "LName";}
puts $fupd1 {  midx "fu1_lfname" 512:case "LName","FName";}
puts $fupd1 "\};"
puts $fupd1 ""
close $fupd1

set clock1 [dbtime]

set status [file exists flogupd1.db]
if {$status} {
  file delete -force flogupd1.db
  file delete -force flogupd1.db.LCK
  file delete -force fu1_ssnidx.inx
  file delete -force fu1_ssnidx.idx
  file delete -force fu1_lnidx.inx
  file delete -force fu1_lnidx.idx
  file delete -force fu1_lfname.inx
  file delete -force fu1_lfname.idx
}

if [catch {dbcreate flogupd1.df} result] {
  puts "Creating table/indexes from flogupd1.df: $result"
  exit 1
}

if [catch {set table [dbopen flogupd1.db]} result] {
  puts "Opening flogupd1.db: $result"
  dbexit
  exit 1
}
if {$table == ""} {
  puts "Opening flogupd1.db: opened by another process."
  exit 1
}

if [catch {set file [open testAdd.txt]} result] {
  puts "Opening file testAdd.txt: $result"
  dbexit
  exit 1
}

if [catch {set lineList [split [read $file] "\n"]} result] {
  puts "Could not get the contents of testAdd.txt: $result"
  dbexit
  exit 1
}
close $file
file delete -force flogupd1.df

set counter 0
foreach line $lineList {
  incr counter
  set output [expr $counter%1000]
  if {$output == 0} {
    puts -nonewline .
    flush stdout
  }
  if {$line != ""} {
    if [catch {set itemList [split $line ":"]} result] {
      puts "Could not split line \"$line\": $result"
      dbexit
      exit 1
    }
    # LName
    set LName [lindex $itemList 0]
    if {$LName == ""} {
      puts "***Warning: LName blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table LName $LName} result] {
      puts "dbsetchar for LName: $result"
      dbexit
      exit 1
    }
    set SSNumber [lindex $itemList 1]
    if {$SSNumber == ""} {
      puts "***Warning: SSNumber blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table SSNumber $SSNumber} result] {
      puts "dbsetchar for SSNumber: $result"
      dbexit
      exit 1
    }
    set FName [lindex $itemList 2]
    if {$FName == ""} {
      puts "***Warning: FName blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table FName $FName} result] {
      puts "dbsetchar for FName: $result"
      dbexit
      exit 1
    }
    set StreetAddress [lindex $itemList 3]
    if {$StreetAddress == ""} {
      puts "***Warning: StreetAddress blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table StreetAddress $StreetAddress} result] {
      puts "dbsetchar for StreetAddress: $result"
      dbexit
      exit 1
    }
    set City [lindex $itemList 4]
    if {$City == ""} {
      puts "***Warning: City is blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table City $City} result] {
      puts "dbsetchar for City: $result"
      dbexit
      exit 1
    }
    set State [lindex $itemList 5]
    if {$State == ""} {
      puts "***Warning: State is blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table State $State} result] {
      puts "dbsetchar for State: $result"
      dbexit
      exit 1
    }
    set Zip [lindex $itemList 6]
    if {$Zip == ""} {
      puts "***Warning: Zip is blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table Zip $Zip} result] {
      puts "dbsetchar for Zip: $result"
      dbexit
      exit 1
    }
    set AnnualSalary [lindex $itemList 7]
    if {$AnnualSalary == ""} {
      puts "***Warning: AnnualSalary is blank in line $counter"
      continue
    }
    if [catch {dbsetnum $table AnnualSalary $AnnualSalary} result] {
      puts "dbsetchar for AnnualSalary to $AnnualSalary: $result"
      dbexit
      exit 1
    }
    set HomeOwner [lindex $itemList 8]
    if {$HomeOwner == ""} {
      puts "***Warning: HomeOwner is blank in line $counter"
      continue
    }
    if [catch {dbsetlog $table HomeOwner $HomeOwner} result] {
      puts "dbsetchar for HomeOwner: $result"
      dbexit
      exit 1
    }
    #
    # Just set DateApplied to the current information.
    #
    if [ catch { dbsetdate $table DateApplied 0 } result ] {
      puts "dbsetdate for DateApplied: $result"
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
    if [catch {dbadd $table} result] {
      if {$result == "adding record: unique index constraint violated"} {
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

if [ catch { set table [ dbopen flogupd1.db ] } result ] {
  puts stderr "Opening flogupd1.db: $result"
  exit 1
}
if {$table == ""} {
  puts "Opening flogupd1.db: opened by another process."
  exit 1
}

set clock2 [dbtime]
set nowTime [expr $clock2-$clock1]
set thisTime [format "%6.4f" $nowTime]

set num_table [dbnumrecs $table]
puts ""
puts "Finished adding to $table: $num_table records in $thisTime seconds"
puts ""

#
# OK, open testAdd2.txt and do the updates.
#
dbhead $table
unset lineList
if [catch {set file [open testAdd2.txt]} result] {
  puts "Opening file testAdd.txt: $result"
  dbexit
  exit 1
}

if [catch {set lineList [split [read $file] "\n"]} result] {
  puts "Could not get the contents of testAdd2.txt: $result"
  dbexit
  exit 1
}
close $file

set clock1 [dbtime]
set counter 0
foreach line $lineList {
  incr counter
  set status [expr $counter%1000]
  if {$status == 0} {
    puts -nonewline "."
    flush stdout
  }
  if {$line != ""} {
    if [catch {set itemList [split $line ":"]} result] {
      puts "Could not split line \"$line\": $result"
      dbexit
      exit 1
    }
    # LName
    set LName [lindex $itemList 0]
    if {$LName == ""} {
      puts "***Warning: LName blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table LName $LName} result] {
      puts "dbsetchar for LName: $result"
      dbexit
      exit 1
    }
    set SSNumber [lindex $itemList 1]
    if {$SSNumber == ""} {
      puts "***Warning: SSNumber blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table SSNumber $SSNumber} result] {
      puts "dbsetchar for SSNumber: $result"
      dbexit
      exit 1
    }
    set FName [lindex $itemList 2]
    if {$FName == ""} {
      puts "***Warning: FName blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table FName $FName} result] {
      puts "dbsetchar for FName: $result"
      dbexit
      exit 1
    }
    set StreetAddress [lindex $itemList 3]
    if {$StreetAddress == ""} {
      puts "***Warning: StreetAddress blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table StreetAddress $StreetAddress} result] {
      puts "dbsetchar for StreetAddress: $result"
      dbexit
      exit 1
    }
    set City [lindex $itemList 4]
    if {$City == ""} {
      puts "***Warning: City is blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table City $City} result] {
      puts "dbsetchar for City: $result"
      dbexit
      exit 1
    }
    set State [lindex $itemList 5]
    if {$State == ""} {
      puts "***Warning: State is blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table State $State} result] {
      puts "dbsetchar for State: $result"
      dbexit
      exit 1
    }
    set Zip [lindex $itemList 6]
    if {$Zip == ""} {
      puts "***Warning: Zip is blank in line $counter"
      continue
    }
    if [catch {dbsetchar $table Zip $Zip} result] {
      puts "dbsetchar for Zip: $result"
      dbexit
      exit 1
    }
    set AnnualSalary [lindex $itemList 7]
    if {$AnnualSalary == ""} {
      puts "***Warning: AnnualSalary is blank in line $counter"
      continue
    }
    if [catch {dbsetnum $table AnnualSalary $AnnualSalary} result] {
      puts "dbsetchar for AnnualSalary to $AnnualSalary: $result"
      dbexit
      exit 1
    }
    set HomeOwner [lindex $itemList 8]
    if {$HomeOwner == ""} {
      puts "***Warning: HomeOwner is blank in line $counter"
      continue
    }
    if [catch {dbsetlog $table HomeOwner $HomeOwner} result] {
      puts "dbsetchar for HomeOwner: $result"
      dbexit
      exit 1
    }
    #
    # Just set DateApplied to the current information.
    #
    if [ catch { dbsetdate $table DateApplied 0 } result ] {
      puts "dbsetdate for DateApplied: $result"
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
    # Now, update them.
    #
    if [catch {set status [dbupdate $table]} result] {
      if {$result == "updating record: index: unique constraint violated"} {
        puts ""
        puts "***Warning: unique constraint violated in update"
	dbclose $table
	if [catch {set status [dbreindex flogupd1.db]} result] {
	  puts "Reindexing flogupd1.db: $result"
	  dbexit
	  exit 1
	} else {
	  puts "flogupd1.db should be OK - reindex was successful\n"
	  exit 0
	}
      }
      puts "Calling dbupdate: $result"
      dbexit
      exit 1
    }
    dbnext $table
  }
}
set clock2 [dbtime]
set nowTime [expr $clock2-$clock1]
set thisTime [format "%6.4f" $nowTime]

set num_table [dbnumrecs $table]
puts ""
puts "Finished adding to $table: $num_table records in $thisTime seconds"
puts ""

dbexit
exit 0
