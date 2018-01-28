#!/bin/sh
# Start it with sh... \
  exec tclsh "$0" "$@"

#
# flogverify1.tcl
#
# Here, I copied over fido and am using that and fidod as the basis for what follows.
# Basically, I am setting up a set of conditions by which I can exhaustively test the
# basic functionality provided by fido/fidod.  I copy some files into a staging directory,
# create some other files destined to be changes and deletes, then get a snapshot of
# the directory.  Then, I delete the delete files, change the change files and add some
# new files.  I then get a second snapshot and compare the old (original one) with the
# new (just as fidod does).  Then, I verify that the deletes, adds and changes are
# correct.
#
# The number of files that get processed (adds, deletes and changes) is generated
# by a psuedo-random generator.
#

#
# Load the shared object.
#
set SLE [info sharedlibextension]
if [ catch { load ../../lib/libcdbtcl$SLE } result ] {
  puts "flogverify1.tcl: could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

set cSourceFileList [ list block.c cdbadd.c cdb.c cdbdf.c \
			cdbedit.c cdbindex.c cdbmove.c cdbpack.c \
			cdbtable.c cdbui.c cdbutils.c cdbws.c \
			idxblk.c \
			index.c qsort.c sort.c \
			test.c token.c words.c ]

set hSourceFileList [ list sort.h block.h blowfish.h test.h mtoken.h win32Util.h \
			cdb.h cdbpp.h index.h \
			interface.h md32_common.h mgetopt.h ]

proc RandomInit { seed } {
  global randomSeed
  set randomSeed $seed
}
proc Random { } {
  global randomSeed
  set randomSeed [ expr ($randomSeed*9301 + 49297) % 233280 ]
  return [ expr $randomSeed / double(233280) ]
}
proc RandomRange { range } {
  expr int ( [ Random ] * $range )
}
#
# OK, seed the generator with some bogus information.
#
set this_pid [ pid ]
set clk [ clock seconds ]
set seed [ expr $clk%$this_pid ]
RandomInit $seed

#
# Create the tables and the data files.
#

puts ""
puts "Creating the tables"
puts ""

if [ catch { set fp [ open flogverify1.df w 0644 ] } result ] {
  puts "Could not create flogverify1.df: $result"
  exit 1
}
puts $fp {// flogverify1.df}
puts $fp ""
puts $fp {// fv1_old.db}
puts $fp {create table "fv1_old.db"}
puts $fp {  info "fv1_old.db: mimics an already existing snapshot of a filesystem (fido)."}
puts $fp "{"
puts $fp {  "fspec" char (128);}
puts $fp {  "uid" char (10);}
puts $fp {  "gid" char (10);}
puts $fp {  "size" number (12:0);}
puts $fp {  "mode" char(10);}
puts $fp {  "mtime" char(19);}
puts $fp {  "md5sum" char (32);}
puts $fp {  "sha1sum" char (40);}
puts $fp {  "ripesum" char (40);}
puts $fp "\} indexed \{"
puts $fp {  idx "fv1_old" 2048:case:unique "fspec";}
puts $fp "\};"
puts $fp ""
puts $fp {//fv1_new.db}
puts $fp {create table "fv1_new.db"}
puts $fp {  info "fv1_new.db: mimics a new snapshot of a filesystem (fido)."}
puts $fp "{"
puts $fp {  "fspec" char (128);}
puts $fp {  "uid" char (10);}
puts $fp {  "gid" char (10);}
puts $fp {  "size" number (12:0);}
puts $fp {  "mode" char(10);}
puts $fp {  "mtime" char(19);}
puts $fp {  "md5sum" char (32);}
puts $fp {  "sha1sum" char (40);}
puts $fp {  "ripesum" char (40);}
puts $fp "\} indexed \{"
puts $fp {  idx "fv1_new" 2048:case:unique "fspec";}
puts $fp "\};"
puts $fp ""
close $fp

file delete -force fv1_old.db
file delete -force fv1_old.db.LCK
file delete -force fv1_old.idx
file delete -force fv1_old.inx
file delete -force fv1_new.db
file delete -force fv1_new.db.LCK
file delete -force fv1_new.idx
file delete -force fv1_new.inx

if [ catch { dbcreate flogverify1.df } result ] {
  puts "Could not create tables: $result"
  exit 1
}
file delete -force flogverify1.df

proc open_table { table_name } {
  if [ catch { set table [ dbopen $table_name ] } result ] {
    puts "Could not open table $table_name: $result"
    return none
  }
  if {$table == ""} {
    puts "Could not open $table_name: opened by another process."
    return none
  }
  return $table
}

proc add_rec { table number fspec uid gid size mode mtime md5sum sha1sum ripesum} {
  if [ catch { dbsetchar $table fspec $fspec } result ] {
    puts "line number $number, dbsetchar $table fspec $fspec: $result"
    return 1
  }
  if [ catch { dbsetchar $table uid $uid } result ] {
    puts "line number $number, dbsetchar $table uid $uid: $result"
    return 1
  }
  if [ catch { dbsetchar $table gid $gid } result ] {
    puts "line number $number, dbsetchar $table gid $gid: $result"
    return 1
  }
  if [ catch { dbsetnum $table size $size } result ] {
    puts "line number $number, dbsetnum $table gid $gid: $result"
    return 1
  }
  if [ catch { dbsetchar $table mode $mode } result ] {
    puts "line number $number, dbsetchar $table mode $mode: $result"
    return 1
  }
  if [ catch { dbsetchar $table mtime $mtime } result ] {
    puts "line number $number, dbsetchar $table mtime $mtime: $result"
    return 1
  }
  if [ catch { dbsetchar $table md5sum $md5sum } result ] {
    puts "line number $number, dbsetchar $table md5sum $md5sum: $result"
    return 1
  }
  if [ catch { dbsetchar $table sha1sum $sha1sum } result ] {
    puts "line number $number, dbsetchar $table sha1sum $sha1sum: $result"
    return 1
  }
  if [ catch { dbsetchar $table ripesum $ripesum } result ] {
    puts "line number $number, dbsetchar $table ripesum $ripesum: $result"
    return 1
  }
  #
  # Add the record.
  #
  if [ catch { dbadd $table } result ] {
    if { $result == "adding record: unique index constraint violated" } {
      return 2
    }
  }
  return $result
}

set old [ open_table fv1_old.db ]
if { $old == "none" } {
  dbexit
  exit 1
}
set new [ open_table fv1_new.db ]
if { $new == "none" } {
  dbexit
  exit 1
}

#
# Source flogverify1_inc.tcl.
# set numIteration ??
if [ catch { source flogverify1_inc.tcl } result ] {
  puts "Could not source flogverify1_inc.tcl: $result"
  exit 1
}

#
# fileModeType (mode, type)
#
# Returns a string of the type -rwxrwxrwx (similar to that returned by ls -l)
# for the file whose mode is mode (given by stat(mode)) and whose type is type
# given by stat(type).

proc fileModeType { mode type } {
  #
  # OK, we are going to look at the mode value and do some bit-flipping
  # to determine what type of file/directory this is.  What follows are
  # masks (in hex) and what value they return when applied against the
  # mode.
  # 
  #                                    User    Grp     Other
  #             ..._ _ _ _ _ _ _ _ _ | _ _ _ | _ _ _ | _ _ _
  #
  # 0xfffffe &                       |                      |
  # 0x0001                                                | |
  # 0x0002                                              | |
  # 0x0004                                            | |
  # 0x0008                                        | |
  # 0x0010                                      | |
  # 0x0020                                    | |
  # 0x0040                                | |
  # 0x0080                              | |
  # 0x0100                            | |
  # 0x800   &   Gives whether SUID
  # 0x400   &   Gives whether GUID
  # 0x200   &   Gives whether Sticky bit on
  #
  # Then, I can use 'file type' and 'file attributes' to get the rest of
  # the info I need.
  # 

  #
  # Get the "other" information
  #
  set mask [ expr $mode & 0x0001 ]
  if { $mask == 1 } {
    set o_x x
  } else {
    set o_x -
  }
  set mask [ expr $mode & 0x0002 ]
  if { $mask == 2 } {
    set o_w w
  } else {
    set o_w -
  }
  set mask [ expr $mode & 0x0004 ]
  if { $mask == 4 } {
    set o_r r
  } else {
    set o_r -
  }
  #
  # check for sticky bit
  #
  set mask [ expr $mode & 0x0200 ]
  if { $mask == 512 } {
    if { $o_x == "x" } {
      set o_x t
    } else {
      set o_x T
    }
  }
  #
  # Group information
  #
  set mask [ expr $mode & 0x0008 ]
  if { $mask == 8 } {
    set g_x x
  } else {
    set g_x -
  }
  set mask [ expr $mode & 0x0010 ]
  if { $mask == 16 } {
    set g_w w
  } else {
    set g_w -
  }
  set mask [ expr $mode & 0x0020 ]
  if { $mask == 32 } {
    set g_r r
  } else {
    set g_r -
  }
  #
  # Check for guid
  #
  set mask [ expr $mode & 0x0400 ]
  if { $mask == 1024 } {
    if { $g_x == "x" } {
      set g_x s
    } else {
      set g_x S
    }
  }
  #
  # User information
  #
  set mask [ expr $mode & 0x0040 ]
  if { $mask == 64 } {
    set u_x x
  } else {
    set u_x -
  }
  set mask [ expr $mode & 0x0080 ]
  if { $mask == 128 } {
    set u_w w
  } else {
    set u_w -
  }
  set mask [ expr $mode & 0x0100 ]
  if { $mask == 256 } {
    set u_r r
  } else {
    set u_r -
  }
  #
  # Check for suid
  #
  set mask [ expr $mode & 0x0800 ]
  if { $mask == 2048 } {
    if { $u_x == "x" } {
      set u_x s
    } else {
      set u_x S
    }
  }
  #
  # See if it is a directory, regular file or some other kind of file
  #
  switch -exact -- $type {
    file { set f_d - }
    directory { set f_d d }
    characterSpecial { set f_d c }
    blockSpecial { set f_d b }
    fifo { set f_d f }
    link { set f_d l }
    socket { set f_d s }
    default { set f_d ? }
  }
  set mode_str [ format "%s%s%s%s%s%s%s%s%s%s" $f_d $u_r $u_w $u_x $g_r $g_w $g_x $o_r $o_w $o_x ]
  return $mode_str
}

set clock1 [ dbtime ]

#
# Create some stock files to check.
#
set pwd [ pwd ]
cd ../../lib
set ni [ format "%07d" $numIteration ]
foreach file $cSourceFileList {
  if { $file != "" } {
    if [ catch { eval exec cp $file ../tests/tcl/bogus_data/$ni.$file } result ] {
      puts "Could not copy $file to ../tests/tcl/bogus_data: $result"
      continue
    }
  }
}
cd ../include
foreach file $hSourceFileList {
  if { $file != "" } {
    if [ catch { eval exec cp $file ../tests/tcl/bogus_data/$ni.$file } result ] {
      puts "Could not copy $file to ../tests/tcl/bogus_data: $result"
      continue
    }
  }
}
cd $pwd

#
# OK, now we have the base files copied over.  Let's generate the change and
# delete files.
#
set numFiles [ RandomRange 20 ]
if { $numFiles == 0 } {
  set numFiles 3
}

set numLines [ RandomRange 100 ]
if { $numLines == 0 } {
  set numLines 30
}

set initialChangeList [ list ]
set initialDeleteList [ list ]

for { set i 0 } { $i < $numFiles } { incr i 1 } {
  if [ catch { set file [ open bogus_data/change_$i.$ni w 0644 ] } result ] {
    puts "Could not create bogus_data/change_$i.$ni: $result"
    continue
  }
  for { set j 0 } { $j < $numLines } { incr j 1 } {
    puts $file "This is a bogus change file"
  }
  close $file
  lappend initialChangeList bogus_data/change_$i.$ni
  if [ catch { set file [ open bogus_data/delete_$i.$ni w 0644 ] } result ] {
    puts "Could not create bogus_data/delete_$i.$ni: $result"
    continue
  }
  for { set j 0 } { $j < $numLines } { incr j 1 } {
    puts $file "This is a bogus delete file"
  }
  close $file
  lappend initialDeleteList bogus_data/delete_$i.$ni
}
if [ catch { eval exec find bogus_data > flogverify1_list.txt } result ] {
  puts "Could not find the stuff in bogus_data -> flogverify1_list.txt: $result"
  exit 1
}

if [ catch { set file [ open flogverify1_list.txt r ] } result ] {
  "Could not open flogverify1_list.txt: $result"
  exit 1
}
if [ catch { set fileList [ split [ read $file ] "\n" ] } result ] {
  "Could not read in flog1verify1_list.txt: $result"
  exit 1
}
close $file

set num_list_items [ llength $fileList ]
puts ""
puts "Processing $num_list_items files"
puts ""
puts ""

set counter 0
foreach item $fileList {
  if { $item != "" } {
    incr counter 1
    set status [ expr $counter%500 ]
    if { $status == 0 } {
      puts -nonewline "."
      flush stdout
    }
    set print 0
    if [ catch { file stat $item fstat } result ] {
      puts "Could not stat $item: $result"
      continue
    }
    #
    # This isn't really necessary right now; however, I thought I would leave
    # it in, just in case I decided later to throw some curve balls at the
    # system by way of testing.
    #
    if { $fstat(type) == "fifo" } {
      puts ""
      puts "$item is a fifo...skipping."
      continue
    }
    if { $fstat(type) == "socket" } {
      puts ""
      puts "$item is a socket...skipping."
      continue
    }
    if { $fstat(type) == "characterSpecial" } {
      puts ""
      puts "$item is a character special device...skipping"
      continue
    }
    if { $fstat(type) == "blockSpecial" } {
      puts ""
      puts "$item is a block special device...skipping"
      continue
    }
    #
    # end fifo, socket, character special, block special
    #
    if [ catch { set md5num [ md5sum "$item" ] } result ] {
      puts "Could not md5sum $item: $result"
      continue
    } else {
      set print 1
    }
    if [ catch { set sha1num [ sha1sum "$item" ] } result ] {
      set print 0
      puts "Could not sha1sum $item: $result"
      continue
    }
    if [ catch { set ripenum [ rmd160sum "$item" ] } result ] {
      set print 0
      puts "Could not rmd160sum $item: $result"
      continue
    }

    #
    # Added to avoid the use of LS
    #
    set perms [ fileModeType $fstat(mode) $fstat(type) ]
    set attrs [ file attributes $item ]
    set user [ lindex $attrs 3 ]
    set grp [ lindex $attrs 1 ]
    set ctime [ clock format $fstat(ctime) -format "%Y/%m/%d-%H:%M:%S" ]
    if { $print == 1 } {
      # Add the data to the $old database.
      set status [ add_rec $old $counter $item $user $grp $fstat(size) \
		     $perms $ctime $md5num $sha1num $ripenum ]
      if { $status == 0 || $status == 1 || $status == 2 } {
	continue
      } else {
	puts "Error adding to $old: $status"
	exit 1
      }
    }
  }
}


#
# OK, now let's change the change files and delete the delete files and add the adds.
set numFiles [ RandomRange 20 ]
if { $numFiles == 0 } {
  set numFiles 3
}
set numLines [ RandomRange 100 ]
if { $numLines == 0 } {
  set numLines 30
}

set initialAddList [ list ]

for { set i 0 } { $i < $numFiles } { incr i 1 } {
  if [ catch { set file [ open bogus_data/add_$i.$ni w 0644 ] } result ] {
    puts "Could not create bogus_data/add_$i.$ni: $result"
    continue
  }
  for { set j 0 } { $j < $numLines } { incr j 1 } {
    puts $file "This is a bogus add file"
  }
  close $file
  lappend initialAddList bogus_data/add_$i.$ni
}
foreach delete $initialDeleteList {
  if [ catch { file delete $delete } result ] {
    puts "Could not delete $delete: $result"
  }
  set status [ file exists $delete ]
  if { $status } {
    puts "WARNING: Could not delete $delete."
  }
}
foreach change $initialChangeList {
  if [ catch { set file [ open $change w 0644 ] } result ] {
    puts "Could not create new $change: $result"
    continue
  }
  for { set j 0 } { $j < $numLines } { incr j 1 } {
    puts $file "This is the new and improved bogus change file."
  }
  close $file
}

if [ catch { eval exec find bogus_data > flogverify1_list.txt } result ] {
  puts "Could not find the stuff in bogus_data -> flogverify1_list.txt: $result"
  exit 1
}

if [ catch { set file [ open flogverify1_list.txt r ] } result ] {
  "Could not open flogverify1_list.txt: $result"
  exit 1
}
unset fileList
if [ catch { set fileList [ split [ read $file ] "\n" ] } result ] {
  "Could not read in flog1verify_list.txt: $result"
  exit 1
}

set num_list_items [ llength $fileList ]
puts ""
puts "Processing $num_list_items files"
puts ""
puts ""

set counter 0
foreach item $fileList {
  if { $item != "" } {
    incr counter 1
    set status [ expr $counter%500 ]
    if { $status == 0 } {
      puts -nonewline "."
      flush stdout
    }
    set print 0
    if [ catch { file stat $item fstat } result ] {
      puts "Could not stat $item: $result"
      continue
    }
    #
    # This isn't really necessary right now; however, I thought I would leave
    # it in, just in case I decided later to throw some curve balls at the
    # system by way of testing.
    #
    if { $fstat(type) == "fifo" } {
      puts ""
      puts "$item is a fifo...skipping."
      continue
    }
    if { $fstat(type) == "socket" } {
      puts ""
      puts "$item is a socket...skipping."
      continue
    }
    if { $fstat(type) == "characterSpecial" } {
      puts ""
      puts "$item is a character special device...skipping"
      continue
    }
    if { $fstat(type) == "blockSpecial" } {
      puts ""
      puts "$item is a block special device...skipping"
      continue
    }
    #
    # end fifo, socket, character special, block special
    #
    if [ catch { set md5num [ md5sum "$item" ] } result ] {
      puts "Could not md5sum $item: $result"
      continue
    } else {
      set print 1
    }
    if [ catch { set sha1num [ sha1sum "$item" ] } result ] {
      set print 0
      puts "Could not sha1sum $item: $result"
      continue
    }
    if [ catch { set ripenum [ rmd160sum "$item" ] } result ] {
      set print 0
      puts "Could not rmd160sum $item: $result"
      continue
    }

    #
    # Added to avoid the use of LS
    #
    set perms [ fileModeType $fstat(mode) $fstat(type) ]
    set attrs [ file attributes $item ]
    set user [ lindex $attrs 3 ]
    set grp [ lindex $attrs 1 ]
    set ctime [ clock format $fstat(ctime) -format "%Y/%m/%d-%H:%M:%S" ]
    if { $print == 1 } {
      # Add the data to the $old database.
      set status [ add_rec $new $counter $item $user $grp $fstat(size) \
		     $perms $ctime $md5num $sha1num $ripenum ]
      if { $status == 0 || $status == 1 || $status == 2 } {
	continue
      } else {
	puts "Error adding to $new: $status"
	exit 1
      }
    }
  }
}

set finalAddList [ list ]
set finalDeleteList [ list ]
set finalChangeList [ list ]

#
# The following are slightly modified versions of the ones from fidod.
#
proc process_adds { newdb olddb } {
  global finalAddList

  #
  # Search for stuff in newdb that is not in olddb.
  #
  dbcurrent $newdb fv1_new
  dbcurrent $olddb fv1_old
  dbheadindex $newdb

  #
  # Traverse through the new database one record at a time.
  #
  set is_adds 0
  while { 1 } {
    set result [ dbiseof $newdb ]
    if { $result == 1 } {
      break
    }
    set result [ dbretrieve $newdb ]
    if { $result == 0 } {
      # There should not be any deleted items, but skip
      # them anyway.
      dbnextindex $newdb
      continue
    }
    set new_spec [ dbshow $newdb fspec ]
    set new_mtime [ dbshow $newdb mtime ]
    set status [ dbsearchexact $olddb $new_spec ]
    if { $status == 0 } {
      incr is_adds
      lappend finalAddList $new_spec
      #puts "is_adds is $is_adds..."
    }
    dbnextindex $newdb
  }
  if { ! $is_adds } {
    puts ""
    puts "WARNING: no adds!"
  } else {
    puts ""
    puts "$is_adds adds processed"
  }
}

proc process_deletes { newdb olddb } {
  global finalDeleteList

  #
  # Search for stuff in olddb that is not in newdb.
  #
  dbcurrent $newdb fv1_new
  dbcurrent $olddb fv1_old
  dbheadindex $olddb

  #
  # Traverse through the old database one record at a time
  #
  set is_deletes 0
  while { 1 } {
    set result [ dbiseof $olddb ]
    if { $result == 1 } {
      break
    }
    set result [ dbretrieve $olddb ]
    if { $result == 0 } {
      # There should not be any deleted items, but skip
      # them anyway.
      dbnextindex $olddb
      continue
    }
    set del_spec [ dbshow $olddb fspec ]
    set status [ dbsearchexact $newdb $del_spec ]
    if { $status == 0 } {
      incr is_deletes
      lappend finalDeleteList $del_spec
    }
    dbnextindex $olddb
  }
  if { ! $is_deletes } {
    puts ""
    puts "WARNING: no deletes!"
  } else {
    puts ""
    puts "$is_deletes deletes processed"
  }
}

proc process_changes { newdb olddb } {
  global finalChangeList

  #
  # Traverse olddb.  For each item there, if it is in newdb, then compare
  # the fields of the database between the two and document any diffs
  # in the changes file.
  #
  dbcurrent $newdb fv1_new
  dbcurrent $olddb fv1_old
  dbheadindex $olddb

  #
  # Traverse the old database.
  #
  set is_changes 0
  while { 1 } {
    set print_changes 0
    set result [ dbiseof $olddb ]
    if { $result == 1 } {
      break
    }
    set result [ dbretrieve $olddb ]
    if { $result == 0 } {
      dbnextindex $olddb
      continue
    }
    set old_fspec [ dbshow $olddb fspec ]
    set status [ dbsearchexact $newdb $old_fspec ]
    #
    # Only deal with it if we found one in both databases.
    #
    #if { $status != 1 } {
    #	puts "Status = $status for fspec $old_fspec"
    #}
    if { $status == 1 } {
      set result [ dbretrieve $newdb ]
      if { $result == 0 } {
	dbnextindex $olddb
	continue
      }
      #
      # Note: mtime is not really the mtime anymore---it is the ctime.
      # The following are the fields that we want to
      # compare (not including fspec):
      # "uid" char 
      # "gid" char 
      # "size" number 
      # "mode" char 
      # "mtime" char
      # "md5sum" char 
      # "sha1sum" char
      # "ripesum" char
      #
      set user_chg 0
      set old_uid [ dbshow $olddb uid ]
      set new_uid [ dbshow $newdb uid ]
      set status [ string compare $old_uid $new_uid ]
      if { $status } {
	set user_chg 1
	incr print_changes
      }
      set grp_chg 0
      set old_gid [ dbshow $olddb gid ]
      set new_gid [ dbshow $newdb gid ]
      set status [ string compare $old_gid $new_gid ]
      if { $status } {
	set grp_chg 1
	incr print_changes
      }
      set size_chg 0
      set old_size [ dbshow $olddb size ]
      set new_size [ dbshow $newdb size ]
      set status [ string compare $old_size $new_size ]
      if { $status } {
	set size_chg 1
	incr print_changes
      }
      set mode_chg 0
      set old_mode [ dbshow $olddb mode ]
      set new_mode [ dbshow $newdb mode ]
      set status [ string compare $old_mode $new_mode ]
      if { $status } {
	set mode_chg 1
	incr print_changes
      }
      set mtime_chg 0
      set old_mtime [ dbshow $olddb mtime ]
      set new_mtime [ dbshow $newdb mtime ]
      set status [ string compare $old_mtime $new_mtime ]
      if { $status } {
	set mtime_chg 1
	#				incr print_changes
      }
      set md5_chg 0
      set old_md5sum [ dbshow $olddb md5sum ]
      set new_md5sum [ dbshow $newdb md5sum ]
      set status [ string compare $old_md5sum $new_md5sum ]
      if { $status } {
	set md5_chg 1
	incr print_changes
      }
      set sha1_chg 0
      set old_sha1sum [ dbshow $olddb sha1sum ]
      set new_sha1sum [ dbshow $newdb sha1sum ]
      set status [ string compare $old_sha1sum $new_sha1sum ]
      if { $status } {
	set sha1_chg 1
	incr print_changes
      }
      set ripe_chg 0
      set old_ripesum [ dbshow $olddb ripesum ]
      set new_ripesum [ dbshow $newdb ripesum ]
      set status [ string compare $old_ripesum $new_ripesum ]
      if { $status } {
	set ripe_chg 1
	incr print_changes
      }
      if { $print_changes } {
	incr is_changes
	lappend finalChangeList $old_fspec
      }
    }
    dbnextindex $olddb
  }
  if { ! $is_changes } {
    puts ""
    puts "WARNING: no changes!"
  } else {
    puts ""
    puts "$is_changes changes processed"
  }
}

process_adds $new $old
process_changes $new $old
process_deletes $new $old

#
# Compare the lists....
#
set initChgList [ lsort -ascii $initialChangeList ]
unset initialChangeList
set initDelList [ lsort -ascii $initialDeleteList ]
unset initialDeleteList
set initAddList [ lsort -ascii $initialAddList ]
unset initialAddList

set finChgList [ lsort -ascii $finalChangeList ]
unset finalChangeList
set finDelList [ lsort -ascii $finalDeleteList ]
unset finalDeleteList
set finAddList [ lsort -ascii $finalAddList ]
unset finalAddList

set chg_num [ llength $finChgList ]
set del_num [ llength $finDelList ]
set add_num [ llength $finAddList ]

set finChg [ lindex $finChgList 0 ]
if { $finChg != "bogus_data" } {
  set fin_inx 0
} else {
  set fin_inx 1
  incr chg_num -1
}

for { set i 0 } { $i < $chg_num } { incr i 1 } {
  set initChg [ lindex $initChgList $i ]
  set index [ expr $i + $fin_inx ]
  set finChg [ lindex $finChgList $index ]
  if { $initChg != $finChg } {
    puts "Changes: \"$initChg\" != \"$finChg\""
    dbexit
    exit 1
  }
}

for { set i 0 } { $i < $del_num } { incr i 1 } {
  set initDel [ lindex $initDelList $i ]
  set finDel [ lindex $finDelList $i ]
  if { $initDel != $finDel } {
    puts "Deletes: $initDel != $finDel"
    dbexit
    exit 1
  }
}

for { set i 0 } { $i < $add_num } { incr i 1 } {
  set initAdd [ lindex $initAddList $i ]
  set finAdd [ lindex $finAddList $i ]
  if { $initAdd != $finAdd } {
    puts "Adds: $initAdd != $finAdd"
    dbexit
    exit 1
  }
}

set clock2 [ dbtime ]
set nowTime [ expr $clock2 - $clock1 ]
set thisTime [ format "%6.4f" $nowTime ]

#
# This doesn't work from cygwin...have to rethink the script or leave it out.
#file delete flogverify1_list.txt -force

puts ""
puts "Finished run in $thisTime seconds"
puts ""
dbexit
exit 0

