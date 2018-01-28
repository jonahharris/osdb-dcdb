echo off
echo Making data...this takes awhile.
c:\tcl\bin\tclsh mkdata.tcl 10000
echo Running flogadd1.tcl...
c:\tcl\bin\tclsh flogadd1.tcl
echo Running flogdel1.tcl...
c:\tcl\bin\tclsh flogdel1.tcl
echo Running flogdel2.tcl...
echo set delNumber 1000 > flogdel2_inc.tcl
c:\tcl\bin\tclsh flogdel2.tcl
echo Running flogdel3.tcl...
echo set delNumber 1000 > flogdel3_inc.tcl
c:\tcl\bin\tclsh flogdel3.tcl
echo Running flogreindex1.tcl...
c:\tcl\bin\tclsh flogreindex1.tcl
echo Running flogcont1.tcl...
c:\tcl\bin\tclsh flogcont1.tcl 25000
echo Running flognum1.tcl...
c:\tcl\bin\tclsh flognum1.tcl 3
echo OK, cleaning up.
del *.db
del *.inx
del *.idx
del testAdd.txt
echo All done.
