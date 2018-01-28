use libcdbpl;

$numItems = $ARGV[0];
if ($numItems == 0) {
	$numItems = 100000;
}

open (TESTPL_DF, ">test3pl.df") || die "***Error: test3pl.df couldn't be created";

print TESTPL_DF '// test3pl.df', "\n";
print TESTPL_DF 'create table "t3_test.db"', "\n";
print TESTPL_DF '  info "Test table 3: test search of int/num values."', "\n";
print TESTPL_DF '{', "\n";
print TESTPL_DF '  "num" number (10:0);', "\n";
print TESTPL_DF '  "fnum" number (10:2);', "\n";
print TESTPL_DF '  "comment" char (80);', "\n";
print TESTPL_DF '} indexed {', "\n";
print TESTPL_DF '  idx "t3_idxnum" 256:case:unique "num";', "\n";
print TESTPL_DF '  idx "t3_idxfnum" 256:case:unique "fnum";', "\n";
print TESTPL_DF '};', "\n";
close TESTPL_DF;

if ( -f "t3_test.db" ) {
	system ("rm -f t3_test.db t3_test.db.LCK t3_idxnum.inx t3_idxnum.idx t3_idxfnum.inx t3_idxnum.idx");
}

eval {
	$status = &libcdbpl::dbcreate ("test3pl.df");
};
if ($@) {
	print ("***Error: dbcreate: " . $@ . "\n");
	exit (1);
}

eval {
	$test = &libcdbpl::dbopen ("t3_test.db");
};
if ($@) {
	print ("***Error: dbopen t3_test.db: " . $@ . "\n");
	exit (1);
}

print ("\nRunning example3.pl...\n\n");

$num = 1;

$clock1 = &libcdbpl::dbtime;

while ($num <= $numItems) {
	&libcdbpl::dbsetint ($test, "num", $num);
	&libcdbpl::dbsetnum ($test, "fnum", $num);
	$comment = &libcdbpl::dbteststring (0, 80);
	&libcdbpl::dbsetchar ($test, "comment", $comment);
	eval {
		&libcdbpl::dbadd ($test);
	};
	if ($@) {
		print ("***Error: dbadd $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit (1);
	}
	$num += 1;
}

$clock2 = &libcdbpl::dbtime;
$nowTime = $clock2-$clock1;
$thisTime = sprintf ("%6.4f", $nowTime);
$num_recs = &libcdbpl::dbnumrecs ($test);

print ("Added $num_recs items in $thisTime seconds\n");

$clock1 = &libcdbpl::dbtime;

#
# Notice that to search for "num" which is defined as:
#
# "num" number (10:0);
#
# you would have to use
#
# $search_item = sprintf ("%10d", 1000);
#
# and search for $search_item.  Integers are stored right justified and padded
# with spaces so that they sort correctly.
#
# If "num" were defined as follows:
#
# "num" number (10:2)
#
# you would have to use 
#
# $search_item = sprintf("%8.2f", 1000);
#
# and search for $search_item (see below for code examples).  Generally, you
# would want to use
#
# sprintf("%<fld>.<dec>f", $number);
#
# for a floating numerical value, where <fld> is given by flen-declen, <dec> is
# declen and $number if the value you are searching for.


&libcdbpl::dbcurrent ($test, "t3_idxnum");
for ($i = 1; $i <= $numItems; $i++)	{
	$search_item = sprintf ("%10d", $i);
	$status = &libcdbpl::dbsearchexact ($test, $search_item);
	if ( ! $status)	{
		print ("\n***Warning: Could not find \"$search_item\" in num index in $test\n");
	}
}

&libcdbpl::dbcurrent ($test, "t3_idxfnum");
for ($i = 1; $i <= $numItems; $i++)	{
	$search_item = sprintf ("%8.2f", $i);
	$status = &libcdbpl::dbsearchexact ($test, $search_item);
	if ( ! $status ) {
		print ("\n***Warning: Could not find \"$search_item\" in fnum index in $test\n");
	}
}

$clock2 = &libcdbpl::dbtime;
$nowTime = $clock2-$clock1;
$thisTime = sprintf ("%6.4f", $nowTime);

print ("Finished searching in $thisTime seconds\n");
&libcdbpl::dbexit;
exit (0);

