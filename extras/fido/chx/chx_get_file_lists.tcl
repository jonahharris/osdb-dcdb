#!/bin/sh
# Start it with sh... \
  exec tclsh "$0" "$@"

#
# Check the command-line.
#
if { $argc == 0 } {
  puts stderr "Improper usage: there must be directories provided on the command-line"
  exit 1
}

#
# Load the shared object.
#
set SLE [info sharedlibextension]
if [catch {load ./libcdbtcl$SLE} result] {
  puts stderr "Could not load ./libcdbtcl$SLE: $result"
  exit 1
}

#
# Traverse through a list of directories, creating a file list from the
# directories.  This file list will be used as a basis for integrity checks
# of individual files.  The directories to traverse through are given as
# items on the command-line.
#

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

#
# Load the configuration file for this architecture.
# ***Change yours to meet your needs.***
#
set OS_TYPE [ eval exec uname ]
if { $OS_TYPE == "SunOS" } {
  set DATE {/bin/date}
  set FIND {/usr/local/bin/find}
  set dt [ eval exec $DATE +%Y%m%d ]
} elseif { $OS_TYPE == "Linux" } {
  set DATE {/bin/date}
  set FIND {/usr/bin/find}
  set dt [ eval exec $DATE +%Y%m%d ]
} else {
  set DATE {date}
  set FIND {find}
  set dt [ eval exec $DATE +%Y%m%d ]
}


set clock1 [ dbtime ]

set CHX_TMP {/tmp/chx_tmp.$dt}

#
# Remove the tmp file if it exists - ignore the results of this.
#
file delete $CHX_TMP

#
# Traverse the directories and generate a list of files to process.
#
foreach dir $argv {
  if { $dir != "" } {
    if [ catch { eval exec $FIND $dir -type f -xdev  >> $CHX_TMP } \
	   error ] {
      puts stderr "Could not run find on $dir: $error"
    }
  }
}

if [ catch { set file [ open /tmp/chx_tmp.$dt r ] } error ] {
  puts stderr "Could not open $CHX_TMP: $error"
  return 1
}

#
# OK, generate a list from the contents of the file.
#
set tmpFileList [ split [ read $file ] "\n" ]

#
# Create fileList as an empty list and add to it the files without pipes in
# the names (clearly, shouldn't happen).  For each one with a ':', print a
# warning that we are ignoring it.
#
set fileList [ list ]
foreach item $tmpFileList {
  set pipestat [ string first "|" $item ]
  if { $pipestat == "-1" } {
    # OK, add to the list
    lappend fileList $item
  } else {
    puts stderr "WARNING: file \"$item\" has a pipe in the name; skipping"
    continue
  }
}

catch { close $file }
file delete $CHX_TMP

set num_list_items [llength $fileList]
puts stderr "Processing $num_list_items files"
puts stderr ""

foreach item $fileList {
  if { $item != "" } {
    set print 0
    if [ catch { file stat $item fstat } result ] {
      puts stderr "Could not stat $item: $result"
      continue
    }
    #
    # We can't handle fifo, socket, char spl or blk spl.
    #
    if { $fstat(type) == "fifo" } {
      puts stderr "$item is a fifo...skipping."
      continue
    }
    if { $fstat(type) == "socket" } {
      puts stderr "$item is a socket...skipping."
      continue
    }
    if { $fstat(type) == "characterSpecial" } {
      puts stderr "$item is a character special device...skipping"
      continue
    }
    if { $fstat(type) == "blockSpecial" } {
      puts stderr "$item is a block special device...skipping"
      continue
    }
    #
    # end fifo, socket, character special, block special
    #
    if [ catch { set md5num [ md5sum "$item" ] } result ] {
      puts stderr "Could not md5sum $item: $result"
      continue
    } else {
      set print 1
    }
    if [ catch { set sha1num [ sha1sum "$item" ] } result ] {
      set print 0
      puts stderr "Could not sha1sum $item: $result"
      continue
    }
    if [ catch { set ripenum [ rmd160sum "$item" ] } result ] {
      set print 0
      puts stderr "Could not rmd160sum $item: $result"
      continue
    }

    set perms [ fileModeType $fstat(mode) $fstat(type) ]
    set attrs [ file attributes $item ]
    set user [ lindex $attrs 3 ]
    set grp [ lindex $attrs 1 ]
    set ctime [ clock format $fstat(ctime) -format "%Y-%m-%d@%H:%M:%S" ]
    if { $print == 1 } {
      set outstr [ format "%s|%s|%s|%s|%d|%s|%s|%s|%s" \
		      $item $perms $user $grp $fstat(size) \
		      $ctime $md5num $sha1num $ripenum ]
      puts "$outstr"
    }
  }
}
puts ""

set clock2 [ dbtime ]
set nowTime [ expr $clock2-$clock1 ]
set thisTime [ format "%6.4f" $nowTime ]

puts stderr ""
puts stderr "Finished run in $thisTime seconds"
puts stderr ""

exit 0
