#!/bin/bash
# \
exec tclsh8.4 "$0" "$@"


set SLE [info sharedlibextension]
if [catch {load /home/dfmay/lib/libcdbtcl$SLE} result] {
  puts "tableshow: could not load /home/dfmay/lib/libcdbtcl$SLE: $result"
  exit 1
}

proc usage { } {
  puts "tableshow <file>"
  puts ""
  puts "  Where <file> is a valid table (.db file)."
  puts ""
  puts "The tableshow command lists information about the table."
}

if {$argc != 1} {
  usage
  exit 1
}

set tbl [lindex $argv 0]

if [catch {set table [dbopen $tbl]} result] {
  puts "tableshow: could not open $tbl: $result"
  exit 1
}
if {$table == ""} {
  puts "tableshow: dbopen $tbl returned NULL"
  exit 1
}


set table_list [list]
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

lappend table_list "Table: $table"
lappend table_list "Number of Records: $num_records"
lappend table_list "Table information: $tbl_info"
lappend table_list { }
set outputstr [ format "%-3s %-50s %-10s %-3s %-3s" {Num} {Field Name} \
  {Type} {Len} {Dec} ]
lappend table_list "$outputstr"
set outputstr [ format "%-3s %-50s %-10s %-3s %-3s" {===} \
  {==================================================} {==========} \
  {===} {===} ]
lappend table_list "$outputstr"

set fnindex [ list ]
set bsindex [ list ]

for { set i 0 } { $i < $num_fields } { incr i } {
  if [ catch { set field_name [ dbfldname $table $i ] } result ] {
    catch { unset table_list }
    set table_list [ list ]
    set return_string [ format "%s %d: %s" {Error calling dbfldname, field} \
      $i $result ]
    puts "***Error: $return_string: $result"
    dbexit
    exit 1
  }
  if [ catch { set isIndexed [ dbisindexed $table $field_name ] } result ] {
    catch { unset table_list }
    set table_list [ list ]
    set return_string [ format "%s %d: %s" {Error calling dbisindexed, \ 
      field} $i $result ]
    puts "***Error: $return_string: $result"
    dbexit
    exit 1
  }
  if { $isIndexed } {
    if [ catch { set blkSize [ dbidxblksz $table $field_name ] } result ] {
      catch { unset table_list }
      set table_list [ list ]
      set return_string [ format "%s %d: %s" {Error calling dbidxblksz, \
        field} $i $result ]
      puts "***Error: $return_string: $result"
      dbexit
      exit 1
    }
    lappend fnindex $field_name
    lappend bsindex $blkSize
  }
  if [ catch { set field_type [ dbfldtype $table $i ] } result ] {
    catch { unset table_list }
    set table_list [ list ]
    set return_string [ format "%s %d: %s" {Error calling dbfldtype, \ 
      field} $i $result ]
    puts "***Error: $return_string: $result"
    dbexit
    exit 1
  }
  if [ catch { set field_len [ dbfldlen $table $i ] } result ] {
    catch { unset table_list }
    set table_list [ list ]
    set return_string [ format "%s %d: %s" {Error calling dbfldlen, \ 
      field} $i $result ]
    puts "***Error: $return_string: $result"
    dbexit
    exit 1
  }
  if [ catch { set field_dec [ dbflddec $table $i ] } result ] {
    catch { unset table_list }
    set table_list [ list ]
    set return_string [ format "%s %d: %s" {Error calling dbflddec, \ 
      field} $i $result ]
    puts "***Error: $return_string: $result"
    dbexit
    exit 1
  }
  set fnum [ expr $i + 1 ]
  set outputstr [ format "%3d %-50s %-10s %3d %3d" $fnum $field_name \
    $field_type $field_len $field_dec ]
  lappend table_list "$outputstr"
}
lappend table_list { }
set numindexes [ llength $fnindex ]
if { $numindexes == 0 } {
  lappend table_list {No field indexes}
} else {
  set outputstr [ format "  %-50s%-5s" {Field Indexes} blksize ]
  lappend table_list "$outputstr"
  set fds [ list ]
  for { set i 0 } { $i < $numindexes } { incr i 1 } {
    set fldnm [ lindex $fnindex $i ]
    set blksz [ lindex $bsindex $i ]
    set outputstr [ format "    %-50s%5d" $fldnm $blksz ]
    lappend table_list "$outputstr"
  }
}
lappend table_list { }
set nmidx [ dbnummidx $table ]
if { ! $nmidx } {
  lappend table_list {  No multi-field indexes}
} else {
  set outputstr [ format "  %-50s%-5s" {Multi-field Indexes} blksz ]
  lappend table_list "$outputstr"
  for { set i 1 } { $i <= $nmidx } { incr i 1 } {
    if [ catch { set indexnm [ dbmidxname $table $i ] } result ] {
      catch { unset table_list }
      set table_list [ list ]
      set return_string [ format "%s %d: %s" {error calling dbmidxname, field} \
        $i $result ]
      puts "***Error: $return_string: $result"
      dbexit
      exit 1
    }
    if [ catch { set blksz [ dbmidxblksz $table $indexnm ] } result ] {
      catch { unset table_list }
      set table_list [ list ]
      set return_string [ format "%s %d: %s" {error calling dbmidxblksz, field} \
        $i $result ]
      puts "***Error: $return_string: $result"
      dbexit
      exit 1
    }
    set outputstr [ format "    %-50s%5d" $indexnm $blksz ]
    lappend table_list "$outputstr"
    # Now, print the fields that are part of this multi-index.
    if [ catch { set fieldnum [ dbmidxnumfldnames $table $indexnm ] } result ] {
      catch { unset table_list }
      set table_list [ list ]
      set return_string [ format "%s %d: %s" \
        {Error calling dbmidxnumfldnames, field} $i $result ]
      puts "***Error: $return_string: $result"
      dbexit
      exit 1
    }
    for { set j 1 } { $j <= $fieldnum } { incr j 1 } {
      if [ catch { set fldnm [ dbmidxfldname $table $indexnm $j ] } result ] {
        catch { unset table_list }
        set table_list [ list ]
        set return_string [ format "%s %d: %s" \
         {Error calling dbmidxnumfldnames, field} $j $result ]
        puts "***Error: $return_string: $result"
          dbexit
        exit 1
      }
      lappend table_list  "    $fldnm"
    }
  }
}
lappend table_list { }

set len [llength $table_list]
if {$len == 0} {
  puts "***Error: table_list is empty"
  dbexit
  exit 1
}
for {set i 0} {$i < $len} {incr i 1} {
  set outstr [lindex $table_list $i]
  puts $outstr
}

dbexit
exit 0
