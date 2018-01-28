#!/bin/sh
# The next line will execute tclsh on this script: \
  exec $TCLSH "$0" "$@"

#
# This script will test the container module, conttcl.so.  This was designed to provde
# the capabilities you get with a generic aggregate data structure (like C's struct) for
# Tcl/Tk or perl.  What's provided in the container module is something similar to the
# DCDB database engine for in-memory storage, with some limitations.  This script will
# test that module.
#

#
# First, process command-line arguments.
#
set verbose 0
if { $argc == 1 } {
  # number is set, verbosity is not
  set numIterations [ lindex $argv 0 ]
} elseif { $argc == 2 } {
  set numIterations [ lindex $argv 0 ]
  set temp [ lindex $argv 1 ]
  if { $temp == "-verbose" } {
    set verbose 1
  }
} else {
  puts -nonewline "Usage: $argv0 "
  puts {iterations [ -verbose ]}
  return 1
}

#
# Now, load the conttcl.{so,dll} and cdbtcl.{so,dll} modules.  These are both needed 
# for this test.
#
set SLE [info sharedlibextension]
if [ catch { load ../../lib/libcdbtcl$SLE } result ] {
  puts "flogcont1.tcl: ouldn't load ../../lib/libcdbtcl$SLE: $result"
  return 1
}

if [ catch { load ../../lib/libconttcl$SLE } result ] {
  puts "flogcont1.tcl: couldn't load ../../lib/libconttcl$SLE: $result"
  return 1
}

proc mkdata { } {
  set LName [ dbtestupperstring 0 64 ]
  set SSNumber [ dbtestnumber 9 0 ]
  set FName [ dbtestupperstring 0 34 ]
  set StreetAddress [ dbteststring 75 0 ]
  set City [ dbteststring 45 0 ]
  set State [ dbtestupperstring 2 0 ]
  set Zip [ dbtestnumber 10 0 ]
  set SalaryIWish [ dbtestnumber 7 0 ]
  set Age [ dbtestnumber 2 0 ]
  set DOBSansCentury [ dbtestnumber 6 0 ]

  set SalaryIWish [ expr int($SalaryIWish)/1 ]
  if { $SalaryIWish < 1000000 } {
    set SalaryIWish [ expr int($SalaryIWish)+1000000 ]
  }
  set DOBSansCentury [ expr int($DOBSansCentury)/1 ]
  if { $DOBSansCentury < 1000000 } {
    set DOBSansCentury [ expr int($DOBSansCentury)+1000000 ]
  }
  set DOB [ expr $DOBSansCentury+19000000 ]
  set AnnualSalary [ expr $SalaryIWish/100.00 ]

  set outstr [ format "%s:%s:%s:%s:%s:%s:%s:%s:%s:%s" \
    $LName $SSNumber $FName $StreetAddress $City $State $Zip \
    $AnnualSalary $Age $DOB ]
  return $outstr
}


#
# Create the container file.
#
set fcont1 [ open flogcont1.cdf w 0644 ]
puts $fcont1 {// flogcont1.cdf}
puts $fcont1 {container "flogcont1"}
puts $fcont1 "{"
puts $fcont1 {  "LName" string (64);}
puts $fcont1 {  "SSNumber" string (9) unique;}
puts $fcont1 {  "FName" string (35);}
puts $fcont1 {  "StreetAddress" string (75);}
puts $fcont1 {  "City" string (45);}
puts $fcont1 {  "State" string (2);}
puts $fcont1 {  "Zip" string (10);}
puts $fcont1 {  "AnnualSalary" float;}
puts $fcont1 {  "Age" int;}
puts $fcont1 {  "DOB" long;}
puts $fcont1 "\}"
close $fcont1

#
# Create the container.
#
set SSNIndex [ list ]
if [ catch { set cont [ containerInit flogcont1.cdf ] } result ] {
  puts "containerInit flogcont1.cdf: $result"
  return 1
}
file delete -force flogcont1.cdf

#
# Write out the data.
#
set outfp [ open flogcont1.output w 0644 ]
for { set i 0 } { $i < $numIterations } { incr i 1 } {
  set line [ mkdata ]
  if { $verbose == 1 } {
    puts "$line"
  }
  puts $outfp $line
}
close $outfp

set infp [ open flogcont1.output r ]
if [ catch { set lineList [ split [ read $infp ] "\n" ] } result ] {
  puts "Could not parse flogcont1.output: $result"
  exit 1
}
close $infp
file delete -force flogcont1.output

set clock1 [ clock clicks -milliseconds ]
set counter 0
foreach line $lineList {
  incr counter
  set output [ expr $counter%1000 ]
  if { $output == 0 } {
    puts -nonewline "."
    flush stdout
  }
  if { $line != "" } {
    if [ catch { set itemList [ split $line ":" ] } result ] {
      puts "Could not split \"$line\": $result"
      return 1
    }
  }
  set LName [ lindex $itemList 0 ]
  if [ catch { containerSetString $cont LName $LName } result ] {
    puts "containerSetString $cont LName $LName: $result"
    containerDelete $cont
    return 1
  }
  set SSNumber [ lindex $itemList 1 ]
  if [ catch { containerSetString $cont SSNumber $SSNumber } result ] {
    puts "containerSetString $cont SSNumber $SSNumber: $result"
    containerDelete $cont
    return 1
  }
  set FName [ lindex $itemList 2 ]
  if [ catch { containerSetString $cont FName $FName } result ] {
    puts "containerSetString $cont FName $FName: $result"
    containerDelete $cont
    return 1
  }
  set StreetAddress [ lindex $itemList 3 ]
  if [ catch { containerSetString $cont StreetAddress $StreetAddress } result ] {
    puts "containerSetString $cont StreetAddress $StreetAddress: $result"
    containerDelete $cont
    return 1
  }
  set City [ lindex $itemList 4 ]
  if [ catch { containerSetString $cont City $City } result ] {
    puts "containerSetString $cont City $City: $result"
    containerDelete $cont
    return 1
  }
  set State [ lindex $itemList 5 ]
  if [ catch { containerSetString $cont State $State } result ] {
    puts "containerSetString $cont State $State: Exp $result"
    containerDelete $cont
    return 1
  }
  set Zip [ lindex $itemList 6 ]
  if [ catch { containerSetString $cont Zip $Zip } result ] {
    puts "containerSetString $cont Zip $Zip: $result"
    containerDelete $cont
    return 1
  }
  set AnnualSalary [ lindex $itemList 7 ]
  if [ catch { containerSetFloat $cont AnnualSalary $AnnualSalary } result ] {
    puts "containerSetFloat $cont AnnualSalary $AnnualSalary: $result"
    containerDelete $cont
    return 1
  }
  set Age [ lindex $itemList 8 ]
  if [ catch { containerSetInt $cont Age $Age } result ] {
    puts "containerSetString $cont Age $Age: $result"
    containerDelete $cont
    return 1
  }
  set DOB [ lindex $itemList 9 ]
  if [ catch { containerSetLong $cont DOB $DOB } result ] {
    puts "containerSetLong $cont DOB $DOB: $result"
    containerDelete $cont
    return 1
  }
  lappend SSNList $SSNumber
  # Add it.
  if [ catch { containerAddRecord $cont } result ] {
    puts ""
    puts "containerAddRecord $cont: $result"
    puts ""
    containerClearError
    # Disregard that entry and continue...
    continue
  }
}

set clock2 [ clock clicks -milliseconds ]
set output [ expr $clock2-$clock1 ]
set runTime [ expr double($output)/1000.00 ]
puts "Added [containerNumRecords $cont] in $runTime seconds."

set clock1 [ clock clicks -milliseconds ]
set len [ llength $SSNList ]
puts "Length of SSN List is $len"
for { set i 0 } { $i < $len } { incr i 1 } {
  set SSNumber [ lindex $SSNList $i ]
  if [ catch {set status [ containerSearch $cont $SSNumber ] } result ] {
    puts "containerSearch $cont $SSNumber: $result"
    return 1
  }
  if { ! $status } {
    puts "***Warning: Could not find $SSNumber"
  }
}

set bogus_list [ list 111111111 222222222 333333333 444444444 555555555 666666666 \
  777777777 888888888 999999999 12321 32123 5852383 ]

foreach bogus_ssn $bogus_list {
  if [ catch { set status [ containerSearch $cont $bogus_ssn ] } result ] {
    puts "containerSearch $cont $SSNumber: $result"
    return 1
  }
  if { $status } {
    puts "***Warning: found something we probably shouldn't have - $bogus_ssn"
  }
}

set clock2 [ clock clicks -milliseconds ]
set output [ expr $clock2-$clock1 ]
set runTime [ expr double($output)/1000.00 ]
puts "Searched [containerNumRecords $cont] in $runTime seconds."

#
# Now, traverse the container and pull the data from it.
#

set clock1 [ clock clicks -milliseconds ]
if { $verbose } {
  puts "Calling containerFirst"
}
if [ catch { set status [ containerFirst $cont ] } result ] {
  "containerFirst $cont: $result"
}
while { 1 == 1 } {
  if { $verbose } {
    puts "getting LName"
  }
  if [ catch { set LName [ containerGetField $cont LName ] } result ] {
    puts "containerGetField $cont $LName: $result"
    break
  }
  if { $verbose } {
    puts "getting SSNumber"
  }
  if [ catch { set SSNumber [ containerGetField $cont SSNumber ] } result ] {
    puts "containerGetField $cont $SSNumber: $result"
    break
  }
  if { $verbose } {
    puts "getting FName"
  }
  if [ catch { set FName [ containerGetField $cont FName ] } result ] {
    puts "containerGetField $cont $FName: $result"
    break
  }
  if { $verbose } {
    puts "getting StreetAddress"
  }
  if [ catch { set StreetAddress [ containerGetField $cont StreetAddress ] } result ] {
    puts "containerGetField $cont $StreetAddress: $result"
    break
  }
  if { $verbose } {
    puts "getting City"
  }
  if [ catch { set City [ containerGetField $cont City ] } result ] {
    puts "containerGetField $cont $City: $result"
    break
  }
  if { $verbose } {
    puts "getting State"
  }
  if [ catch { set State [ containerGetField $cont State ] } result ] {
    puts "containerGetField $cont $State: Exp $result"
    break
  }
  if { $verbose } {
    puts "getting Zip"
  }
  if [ catch { set Zip [ containerGetField $cont Zip ] } result ] {
    puts "containerGetField $cont $Zip: $result"
    break
  }
  if { $verbose } {
    puts "getting AnnualSalary"
  }
  if [ catch { set AnnualSalary [ containerGetField $cont AnnualSalary ] } result ] {
    puts "containerGetField $cont $AnnualSalary: $result"
    break
  }
  if { $verbose } {
    puts "getting Age"
  }
  if [ catch { set Age [ containerGetField $cont Age ] } result ] {
    puts "containerGetField $cont $Age: $result"
    break
  }
  if { $verbose } {
    puts "getting DOB"
  }
  if [ catch { set DOB [ containerGetField $cont DOB ] } result ] {
    puts "containerGetField $cont $DOB: $result"
    break
  }
  if { $verbose } {
    puts "$LName:$SSNumber:$FName:$StreetAddress:$City:$State: Exp $Zip:$AnnualSalary:$Age:$DOB"
  }
  if { $verbose } {
    puts "calling containerEOF"
  }
  set status [ containerNext $cont ]
  set status [ containerEOF $cont ]
  if { $status } {
    break
  }
}
set clock2 [ clock clicks -milliseconds ]
set output [ expr $clock2-$clock1 ]
set runTime [ expr double($output)/1000.00 ]
puts "Read [containerNumRecords $cont] records in $runTime seconds."

if [ catch { containerDelete $cont } result ] {
  puts "containerDelete $cont: $result"
  return 1
}

exit 0

