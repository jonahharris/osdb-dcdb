#!/bin/sh
# The next line will execute tclsh on this script: \
exec $TCLSH "$0" "$@"

#
# First, load the libcdbtcl.so file.
#
set SLE [info sharedlibextension]
if [catch {load ../../lib/libcdbtcl$SLE} result] {
  puts stderr "flogadd1.tcl: could not load ../../lib/libcdbtcl$SLE: $result"
  exit 1
}

proc usage { } {
	puts ""
	puts {flognum2.tcl scale num [+-x/] num}
  puts "where scale is the decimal scale (0-20)"
  puts "and num is a decimal real number"
  puts "The operation indicated will be applied"
  puts "to the numbers in the expected (algebraic) way"
	puts ""
}

if {$argc != 4} {
	puts "Bad command line:"
	puts ""
	usage
	exit 1
}

set scale [lindex $argv 0]
set num1  [lindex $argv 1]
set op    [lindex $argv 2]
set num2  [lindex $argv 3]

switch -exact -- $op {
	{+} \
	{
		if [catch {set val [bcnumadd $num1 $num2 $scale]} result] {
			if {$result == "bcnum error: output string is too small"} {
				#puts stderr "bcnummultiply $num1 $num2 $scale result too large"
				exit 4
			}
			puts "***Error: bcnumadd $num1 $num2 $scale: $result"
			bcnumuninit
			exit -1
		}
		puts "$val"
	}
	{-} \
	{
		if [catch {set val [bcnumsub $num1 $num2 $scale]} result] {
			if {$result == "bcnum error: output string is too small"} {
				#puts stderr "bcnummultiply $num1 $num2 $scale result too large"
				exit 4
			}
			puts "***Error: bcnumsub $num1 $num2 $scale: $result"
			bcnumuninit
			exit -1
		}
		puts "$val"
	}
	{x} \
	{
		if [catch {set val [bcnummultiply $num1 $num2 $scale]} result] {
			if {$result == "bcnum error: output string is too small"} {
				#puts stderr "bcnummultiply $num1 $num2 $scale result too large"
				exit 4
			}
			puts "***Error: bcnummultiply $num1 $num2 $scale: $result"
			bcnumuninit
			exit -1
		}
		puts "$val"
	}
	{/} \
	{
		if [catch {set val [bcnumdivide $num1 $num2 $scale]} result] {
			if {$result == "bcnum error: output string is too small"} {
				#puts stderr "bcnummultiply $num1 $num2 $scale result too large"
				exit 4
			}
			puts "***Error: bcnumdivide $num1 $num2 $scale: $result"
			bcnumuninit
			exit -1
		}
		puts "$val"
	}
	default \
	{
		puts "***Error: invalid operator $op"
		exit -1
	}
}
bcnumuninit
exit 0
