#!/bin/bash
# \
exec tclsh8.4 "$0" "$@"

set SLE [info sharedlibextension]
if [catch {load /home/dfmay/lib/libcdbtcl$SLE} result] {
  puts "tableshow: could not load /home/dfmay/lib/libcdbtcl$SLE: $result"
  exit 1
}

proc usage { } {
  puts "tablels <file>"
  puts ""
  puts "  Where <file> is a valid table (.db file)."
  puts ""
  puts "The tablels command lists the contents of a table."
}

if {$argc != 1} {
  usage
  exit 1
}

set tbl [lindex $argv 0]

if [catch {set table [dbopen $tbl]} result] {
  puts "tablels: could not open $tbl: $result"
  exit 1
}
if {$table == ""} {
  puts "tablels: dbopen $tbl returned NULL"
  exit 1
}

if [ catch { set num_fields [ dbnumfields $table ] } result ] {
  catch { unset table_list }
  set return_string [ format "%s: %s" {Error calling dbnumfields} $result ]
  puts "***Error: $return_string"
  dbexit
  exit 1
}
if [ catch { set num_records [ dbnumrecs $table ] } result ] {
  catch { unset table_list }
  set return_string [ format "%s: %s" {Error calling dbnumrecs} $result ]
  puts "***Error: $return_string"
  dbexit
  exit 1
}
if [catch {set tbl_info [dbshowinfo $table]} result] {
  catch {unset table_list}
  set return_string [format "%s: %s" {Error calling dbshowinfo} $result]
  puts "***Error: $return_string"
  dbexit
  exit 1
}

#puts "Table: $table"
#puts "Number of records: $num_records"
#puts "Number of fields: $num_fields"
#puts "Table information: $tbl_info"

#puts ""
#puts "--------------------------------------------------------------------------------"
#puts ""

if {$num_records == 0} {
  puts "  There are no records in the table"
  dbexit
  exit 0
}

if [catch {set status [dbhead $table]} result] {
  puts "Error calling dbhead: $result"
  dbexit
  exit 1
}

if [catch {set status [dbretrieve $table]} result] {
  puts "Error calling dbretrieve: $result"
  dbexit
  exit 1
}

for {set j 0} {$j < $num_fields} {incr j 1} {
  if [catch {set this_fld [dbfldname $table $j]} result] {
    puts "Error calling dbfldname $table $j: $result"
    dbexit
    exit 1
  }
  set k $j
  incr k
  if {$k < $num_fields} {
    puts -nonewline "\"$this_fld\", "
  } else {
    puts "\"$this_fld\""
  }
}
puts "--------------------------------------------------------------------------------"

for {set i 0} {$i < $num_records} {incr i 1} {
  # Loop through the records.
  for {set j 0} {$j < $num_fields} {incr j 1} {
    # Loop through the fields.
    if [catch {set this_fld [dbfldname $table $j]} result] {
      puts "Error calling dbfldname $table $j: $result"
      dbexit
      exit 1
    }
    if [catch {set fld_val [dbshow $table $this_fld]} result] {
      puts "Error calling dbshow $table $this_fld: $result"
      dbexit
      exit 1
    }
    set k $j
    incr k
    if {$k < $num_fields} {
      puts -nonewline "\"$fld_val\","
    } else {
      puts "\"$fld_val\""
    }
  }
  if [catch {set status [dbnext $table]} result] {
    puts "Error calling dbnext: $result"
    dbexit
    exit 1
  }
  set status [dbiseof $table]
  if {$status} {
    break
  }
  if [catch {set status [dbretrieve $table]} result] {
    puts "Error calling dbretrieve: $result"
    dbexit
    exit 1
  }
}

#puts ""
#puts "--------------------------------------------------------------------------------"
#puts ""

dbexit
exit 0
