#
# Support scripts for DCDB.
#

package provide cdb 1.1

#
# [BeginDoc]
# 
# \section{DCDB Tcl module}
# 
# This is the DCDB Tcl module.  This is provided in addition to the Tcl/Tk
# bindings to provide support for high-level functionality that has been
# shown to be useful in other projects.  There is no corresponding perl module.
# 
# To use this module, you have to first load cdbtcl.so.  Then, you have to
# append the location of the package to auto_path and then use
# ``package require''.  The following is an example (ignore the `#' on each line):
#
# [EndDoc] 
# @DocInclude ../extras/tcl/ex1.txt
# [BeginDoc]
#
# Of course, this assumes that you have built the pkgIndex.tcl file first.
# You can use mk_pkg.sh to do that.  Also, you are not limited to this
# location.  You can put the cdb.tcl file wherever and append its path
# to the auto_path variable.
#
# Everything in this package is in the cdb namespace to avoid conflicts with
# other packages.
#
# [EndDoc]
#

namespace eval cdb {
  # Create the variables and such.
  variable seed [ clock clicks -milliseconds ]
  variable tableListing
  variable tableArray
  variable fileMode
  namespace export randomInit random randomRange
  namespace export isValidTable fileModeString
}

#
# [BeginDoc]
#
# \subsection{random}
# \index{random}
#
# proc cdb::randomInit { seed }
# \index{randomInit}
#
# The random functionality consists of several procedures.  These are pure
# TCL procedures that provide psuedo-random functionality.  The first
# procedure is ``randomInit''.  It is called as follows:
#
# [EndDoc]
# @DocInclude ../extras/tcl/ex2.txt
# [BeginDoc]
# 
# It is not necessary to call this function; the variable ``::cdb::seed'' is set
# to the value of [ clock clicks -milliseconds ] which should be sufficient
# for most purposes.
#
# [EndDoc]
#
proc cdb::randomInit { seed } {
  set ::cdb::seed $seed
}

#
# [BeginDoc]
#
# proc cdb::random { }
#
# The next procedure in random group is called ``random''.  \index{random} This
# function simply calculates a random value between 0 and 1 based on the seed
# that was either provided or calculated from the current time.  It is used
# as follows:
#
# [EndDoc]
# @DocInclude ../extras/tcl/ex3.txt
# [BeginDoc]
#
# If you use the random procedure and want to generate another random value,
# you should reseed with a call to ``cdb::randomInit''.
#
# [EndDoc]
#

proc cdb::random {} {
  variable seed
  set seed [ expr ($seed*9301+49297) % 233280 ]
  return [ expr $seed/double(233280) ]
}

#
# [BeginDoc]
#
# proc cdb::randomRange { range }
# 
# The ``randomRange'' \index{randomRange} procedure is like the random procedure
# except that it returns a psuedo-random integer between 0 and the ``range''
# value provided as an argument.  It is used as follows:
#
# [EndDoc]
# @DocInclude ../extras/tcl/ex4.txt
#

proc cdb::randomRange { range } {
  expr int([random]*$range)
}

#
# [BeginDoc]
#
# \subsection{isValidTable}
# \index{isValidTable}
#
# proc cdb::isValidTable { table }
# 
# The ``isValidTable'' procedure returns 0 (false) if the ``table'' argument is
# either not a valid table handle or is a table handle but the table pointed to by
# it is not open.  If the ``table'' argument is valid and points to an open table,
# ``isValidTable'' returns 1 (true).
# 
# [EndDoc]
#
proc cdb::isValidTable { table } {
  if [ catch { dbisbof $table } result ] {
    return 0
  } else {
    return 1
  }
}

#
# [BeginDoc]
#
# \subsection{listTable}
# \index{listTable}
#
# proc cdb::listTable { table }
#
# The ``listTable'' procedure takes as an argument a valid table handle given by
# ``table''.  If successful, ``listTable'' populates a list called
# ``::cdb::tableListing'' that is part of the cdb namespace with formatted data
# about the list, including number of records, information about the fields in
# the table and information about the indexes that are part of the table.  Then,
# listTable returns 0.
#
# The output of ``listTable'' is pre-formatted by the function in a way that is
# deemed to be attractive.  If the caller wants to format the output in another
# way, the data in the list will need to be extracted first.  There is no provision
# for altering the format of how data is presented.
#
# If the ``table'' argument is not a valid table handle or there is an error,
# the ``::cdb::tableListing'' list will have as a first value a string that
# indicates what the error was and ``listTable'' will return 1.
# 
# Use the listTable procedure as follows:
#
# [EndDoc]
# @DocInclude ../extras/tcl/ex5.txt
#
# TODO: accept another argument, called formatted.  If it is true, format the output;
# if it is false, just populate the list with the output in a way that is easy
# to parsse.
#
proc cdb::listTable { table } {
  variable tableListing
  catch { unset tableListing }
  set status [ isValidTable $table ]
  if { ! $status } {
    set return_string [ format "Table handle %s is invalid\n" $table ]
    set tableListing [ list ]
    lappend tableListing $return_string
    return 1
  }
  set tableListing [ list ]
  if [ catch { set num_fields [ dbnumfields $table ] } result ] {
    catch { unset tableListing }
    set return_string [ format "%s: %s" {Error calling dbnumfields} $result ]
    set tableListing [ list ]
    lappend tableListing $return_string
    return 1
  }
  if [ catch { set num_records [ dbnumrecs $table ] } result ] {
    catch { unset tableListing }
    set return_string [ format "%s: %s" {Error calling dbnumrecs} $result ]
    set tableListing [ list ]
    lappend tableListing $return_string
    return 1
  }
  lappend tableListing "Table: $table"
  lappend tableListing "Number of Records: $num_records"
  lappend tableListing { }
  set outputstr [ format "%-3s %-50s %-10s %-3s %-3s" {Num} {Field Name} \
    {Type} {Len} {Dec} ]
  lappend tableListing "$outputstr"
  set outputstr [ format "%-3s %-50s %-10s %-3s %-3s" {===} \
    {==================================================} {==========} \
    {===} {===} ]
  lappend tableListing "$outputstr"

  set fnindex [ list ]
  set bsindex [ list ]

  for { set i 0 } { $i < $num_fields } { incr i } {
    if [ catch { set field_name [ dbfldname $table $i ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" {Error calling dbfldname, field} \
        $i $result ]
      lappend tableListing $return_string
      return 1
    }
    if [ catch { set isIndexed [ dbisindexed $table $field_name ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" {Error calling dbisindexed, \ 
	field} $i $result ]
      lappend tableListing $return_string
      return 1
    }
    if { $isIndexed } {
      if [ catch { set blkSize [ dbidxblksz $table $field_name ] } result ] {
        catch { unset tableListing }
        set tableListing [ list ]
        set return_string [ format "%s %d: %s" {Error calling dbidxblksz, \
	  field} $i $result ]
        lappend tableListing $return_string
        return 1
      }
      lappend fnindex $field_name
      lappend bsindex $blkSize
    }
    if [ catch { set field_type [ dbfldtype $table $i ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" {Error calling dbfldtype, \ 
	field} $i $result ]
      lappend tableListing $return_string
      return 1
    }
    if [ catch { set field_len [ dbfldlen $table $i ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" {Error calling dbfldlen, \ 
	field} $i $result ]
      lappend tableListing $return_string
      return 1
    }
    if [ catch { set field_dec [ dbflddec $table $i ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" {Error calling dbflddec, \ 
	field} $i $result ]
      lappend tableListing $return_string
      return 1
    }
    set fnum [ expr $i + 1 ]
    set outputstr [ format "%3d %-50s %-10s %3d %3d" $fnum $field_name \
      $field_type $field_len $field_dec ]
    lappend tableListing "$outputstr"
  }
  lappend tableListing { }
  set numindexes [ llength $fnindex ]
  if { $numindexes == 0 } {
    lappend tableListing {No field indexes}
  } else {
    set outputstr [ format "%-50s%-5s" {Field Indexes} blksize ]
    lappend tableListing "$outputstr"
    set fds [ list ]
    for { set i 0 } { $i < $numindexes } { incr i 1 } {
      set fldnm [ lindex $fnindex $i ]
      set blksz [ lindex $bsindex $i ]
      set outputstr [ format "%-50s%5d" $fldnm $blksz ]
      lappend tableListing "$outputstr"
    }
  }
  lappend tableListing { }
  set nmidx [ dbnummidx $table ]
  if { ! $nmidx } {
    lappend tableListing {No multi-field indexes}
  } else {
    set outputstr [ format "%-50s%-5s" {Multi-field Indexes} blksz ]
    lappend tableListing "$outputstr"
    for { set i 1 } { $i <= $nmidx } { incr i 1 } {
    if [ catch { set indexnm [ dbmidxname $table $i ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" {error calling dbmidxname, field} \
        $i $result ]
      lappend tableListing $return_string
      return 1
    }
    if [ catch { set blksz [ dbmidxblksz $table $indexnm ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" {error calling dbmidxblksz, field} \
        $i $result ]
      lappend tableListing $return_string
      return 1
    }
    set outputstr [ format "%-50s%5d" $indexnm $blksz ]
    lappend tableListing "$outputstr"
    # Now, print the fields that are part of this multi-index.
    if [ catch { set fieldnum [ dbmidxnumfldnames $table $indexnm ] } result ] {
      catch { unset tableListing }
      set tableListing [ list ]
      set return_string [ format "%s %d: %s" \
        {Error calling dbmidxnumfldnames, field} $i $result ]
      lappend tableListing $return_string
      return 1
    }
    for { set j 1 } { $j <= $fieldnum } { incr j 1 } {
      if [ catch { set fldnm [ dbmidxfldname $table $indexnm $j ] } result ] {
        catch { unset tableListing }
        set tableListing [ list ]
        set return_string [ format "%s %d: %s" \
	  {Error calling dbmidxnumfldnames, field} $j $result ]
        lappend tableListing $return_string
        return 1
      }
        lappend tableListing  {    $fldnm}
      }
    }
  }
  lappend tableListing { }
  # Everything is OK, return clean return value.
  return 0
}

#
# [BeginDoc]
#
# \subsection{listTableArray}
# \index{listTableArray}
#
# The ``listTableArray'' procedure is similar to the ``listTable'' procedure in that
# it provides information about the table given as the argument ``table''.  However,
# ``listTableArray doesn't provide as much information.  Also, the information is
# not formatted.  The information is returned in an array called ``::cdb::tableArray''.
# The information contained in the ``cdb::tableArray'' array is as follows:
#
# [EndDoc]
# @DocInclude ../extras/tcl/ex6.txt
#

proc cdb::listTableArray { table } {
  variable tableArray

  catch { unset tableArray }
  if [ catch { set num_fields [ dbnumfields $table ] } result ] {
    catch { unset tableArray }
    set return_string [ format "%s: %s" {Error calling dbnumfields} $result ]
    set tableArray(error) $resturn_string
    return 1
  }

  set tableArray(numfields) $num_fields
  for { set i 0 } { $i < $tableArray(numfields) } { incr i 1 } {
    if [ catch { set field_name [ dbfldname $table $i ] } result ] {
      catch { unset tableArray }
      set return_string [ format "%s %d: %s" {Error calling dbfldname, field} \
        $i $result ]
      set tableArray(error) $return_string
      return 1
    }
    set tableArray($i,name) $field_name
    if [ catch { set isIndexed [ dbisindexed $table $field_name ] } result ] {
      catch { unset tableArray }
      set return_string [ format "%s %d: %s" {Error calling dbisindexed, \ 
	field} $i $result ]
      set tableArray(error) $return_string
      return 1
    }
    if { $isIndexed } {
      set tableArray($i,indexed) true
    } else {
      set tableArray($i,indexed) false
    }
    if [ catch { set field_type [ dbfldtype $table $i ] } result ] {
      catch { unset tableArray }
      set return_string [ format "%s %d: %s" {Error calling dbfldtype, \ 
	field} $i $result ]
      set tableArray(error) $return_string
      return 1
    } else {
      set tableArray($i,fieldtype) $field_type
    }
    if [ catch { set field_len [ dbfldlen $table $i ] } result ] {
      catch { unset tableArray }
      set return_string [ format "%s %d: %s" {Error calling dbfldlen, \ 
	field} $i $result ]
      set tableArray(error) $return_string
      return 1
    } else {
      set tableArray($i,fieldlen) $field_len
    }
    if [ catch { set field_dec [ dbflddec $table $i ] } result ] {
      catch { unset tableArray }
      set return_string [ format "%s %d: %s" {Error calling dbflddec, \ 
	field} $i $result ]
      set tableArray(error) $return_string
      return 1
    } else {
      set tableArray($i,fielddec) $field_dec
    }
  }
  return 0
}

#
# [BeginDoc]
#
# \subsection{fileModeString}
# \index{fileModeString}
#
# cdb::fileModeString { fname }
#
# The ``fileModeString'' procedure takes the name of a file as an argument and
# returns a mode string similar to the mode string shown by `ls -l', something
# like `-rw-r--r--' for a normal file with 644 permissions.  The difference
# between `ls -l' and ``fileModeString'' has to do with how ``fileModeString''
# handles symbolic links.  ``fileModeString'' will return the mode string for
# the file linked to, whereas `ls -l' returns lrwxrwxrwx.  I felt the mode string
# for the file being pointed to is more useful than the other.
#
# The file mode is returned in the variable ``::cdb::fileMode'', which is in the
# cdb namespace.  If an error occurs, ``fileModeString'' returns a non-zero value
# and ``cdb::fileMode'' contains the error.  If there is no error, the return value
# will be 0 and ``cdb::fileMode'' will contain the mode string.  ``fileModeString''
# would be used as follows:
#
# [EndDoc]
# @DocInclude ../extras/tcl/ex7.txt
#
proc cdb::fileModeString { fname } {
  variable fileMode
  if [ catch { unset fileMode } result ] {
    # ignore it
  }

  #
  # OK, we are going to look at the mode value and do some bit-flipping
  # to determine what type of file/directory this is.  What follows are
  # masks (in hex) and what value they return when applied against the
  # mode.
  # 
  #                                    User    Grp     Other
  #                                    r w x   r w x   r w x
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
  # First, stat the file to get the mode and type.
  #
  set exists [ file exists $fname ]
  if { ! $exists } {
    catch [ unset fileMode ]
    set fileMode [ format "***Error: file %s does not exist" $fname ]
    return 1
  }
  if [ catch { file stat $fname fstat } result ] {
    catch [ unset fileMode ]
    set fileMode [ format "***Error: could not stat %s" $fname ]
    return 1
  }
  set mode $fstat(mode)
  set type $fstat(type)

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
  set fileMode [ format "%s%s%s%s%s%s%s%s%s%s" $f_d $u_r $u_w $u_x $g_r $g_w $g_x $o_r $o_w $o_x ]
  return 0
}
