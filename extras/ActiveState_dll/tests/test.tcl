#!/bin/sh
# Before you run this, set TCLSH to your version of tclsh (e.g., tclsh8.4).
# The next line will execute tclsh on this script: \
exec $TCLSH "$0" "$@"

set NUMBER_MOD 10
set CHAR_MOD 26
set MIX_MOD 52
set regstr  "abcdefghijklmnopqrstuvwxyz"
set capstr  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
set mixstr  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
set numstr  "1234567890"

#set random_seed 0

#proc rand_init { seed } {
#  global random_seed
#  set random_seed $seed
#}

#proc rand {} {
#  global random_seed
#  set random_seed [expr ($random_seed*9301 + 49297) % 233280]
#  return [expr ($random_seed / double(233280))]
#}

proc rand_int { range } {
  expr int ($range * rand())
}

proc teststring { len maxlen } {
  global NUMBER_MOD
  global CHAR_MOD
  global MIX_MOD
  global regstr
  global capstr
  global mixstr
  global numstr

#  rand_init [clock click -milliseconds]
  set rand_str ""
  if {$len == 0} {
    if {$maxlen == 0} {
      set maxlen 40
    }
    set len [rand_int $maxlen]
    if {$len < 5 && $maxlen > 5} {
      set len 5
    }
  }
  for {set i 0} {$i < $len} {incr i 1} {
    set offst [rand_int $CHAR_MOD]
    set ltr [string range $regstr $offst $offst]
    set rand_str [format "%s%s" $rand_str $ltr]
  }
  return $rand_str
}

proc testupperstring { len maxlen } {
  global NUMBER_MOD
  global CHAR_MOD
  global MIX_MOD
  global regstr
  global capstr
  global mixstr
  global numstr

#  rand_init [clock click -milliseconds]
  set rand_str ""
  if {$len == 0} {
    if {$maxlen == 0} {
      set maxlen 40
    }
    set len [rand_int $maxlen]
    if {$len < 5 && $maxlen > 5} {
      set len 5
    }
  }
  for {set i 0} {$i < $len} {incr i 1} {
    set offst [rand_int $CHAR_MOD]
    set ltr [string range $capstr $offst $offst]
    set rand_str [format "%s%s" $rand_str $ltr]
  }
  return $rand_str
}

proc testnumber { len maxlen } {
  global NUMBER_MOD
  global CHAR_MOD
  global MIX_MOD
  global regstr
  global capstr
  global mixstr
  global numstr

#  rand_init [clock click -milliseconds]
  set rand_str ""
  if {$len == 0} {
    if {$maxlen == 0} {
      set maxlen 40
    }
    set len [rand_int $maxlen]
    if {$len < 5 && $maxlen > 5} {
      set len 5
    }
  }
  for {set i 0} {$i < $len} {incr i 1} {
    set offst [rand_int $NUMBER_MOD]
    set ltr [string range $numstr $offst $offst]
    if {$i == 0 && $ltr == "0"} {
      set ltr "1"
    }
    set rand_str [format "%s%s" $rand_str $ltr]
  }
  return $rand_str
}

proc testmixedstring { len maxlen } {
  global NUMBER_MOD
  global CHAR_MOD
  global MIX_MOD
  global regstr
  global capstr
  global mixstr
  global numstr

#  rand_init [clock click -milliseconds]
  set rand_str ""
  if {$len == 0} {
    if {$maxlen == 0} {
      set maxlen 40
    }
    set len [rand_int $maxlen]
    if {$len < 5 && $maxlen > 5} {
      set len 5
    }
  }
  for {set i 0} {$i < $len} {incr i 1} {
    set offst [rand_int $MIX_MOD]
    set ltr [string range $mixstr $offst $offst]
    set rand_str [format "%s%s" $rand_str $ltr]
  }
  return $rand_str
}


