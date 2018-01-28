use libcdbpl;

#
# First, create the definition file.
#

open (TESTPL_DF, ">test2pl.df") || die "***Error: test2pl.df couldn't be created";

print TESTPL_DF '// test2pl.df', "\n";
print TESTPL_DF 'create table "t2_test.db"', "\n";
print TESTPL_DF '  info "Test table 2: test entry/retrieval of records."',"\n";
print TESTPL_DF '{',"\n";
print TESTPL_DF '  "ssn" char (9);',"\n";
print TESTPL_DF '  "lname" char (30);', "\n";
print TESTPL_DF '  "fname" char (15);', "\n";
print TESTPL_DF '} indexed {', "\n";
print TESTPL_DF '  idx "t2_idxssn" 256:case:unique "ssn";', "\n";
print TESTPL_DF '  midx "t2_midxname" 512:nocase "lname", "fname";', "\n";
print TESTPL_DF '};', "\n";
close TESTPL_DF;

if ( -f "t2_test.db" ) {
	system ("rm -f t2_test.db t2_test.db.LCK t2_idxssn.idx t2_idxssn.inx t2_midxname.idx t2_midxname.inx");
}

eval {
	$status = &libcdbpl::dbcreate ("test2pl.df");
};
if ($@) {
	print ("***Error: dbcreate: " . $@ . "\n");
	exit (1);
}

eval {
	$test = &libcdbpl::dbopen ("t2_test.db");
};
if ($@) {
	print ("***Error: dbopen t2_test.db: " . $@ . "\n");
	exit (1);
}

$iterations = 25000;

$t_ssn = 555555555;
$t_lname1 = $t_ssn + $iterations;
$t_lname2 = "test";
$t_fname = "David";

$clock1 = &libcdbpl::dbtime;

for ( $i = 0; $i < $iterations; $i++)	{
	$name = $t_lname2 . $t_lname1;
	$t_lname1 -= 1;
	&libcdbpl::dbsetchar ($test, "lname", $name);
	&libcdbpl::dbsetchar ($test, "fname", $t_fname);
	&libcdbpl::dbsetchar ($test, "ssn", $t_ssn);
	$t_ssn += 1;
	eval {
		$status = &libcdbpl::dbadd ($test);
	};
	if ($@)	{
		print ("***Error: dbadd: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit (1);
	}
}

$t_ssn = 555555555;
$t_lname1 = $t_ssn + $iterations;

$num_test = &libcdbpl::dbnumrecs ($test);

$clock2 = &libcdbpl::dbtime;
$thisTime = $clock2-$clock1;

print ("Finished adding: $num_test records added in t2_test.db\n");
printf ("Finished in %6.4f seconds\n\n", $thisTime);

&libcdbpl::dbcurrent ($test, "t2_idxssn");
&libcdbpl::dbheadindex ($test);

$status = &libcdbpl::dbiseof ($test);
if ($status == 1)	{
	print ("There are no records in t2_test.db!\n");
	&libcdbpl::dbexit;
	exit (-1);
}

system ("rm -f test2pl.df");

$clock1 = &libcdbpl::dbtime;

print ("SSN index order...\n");
LOOP1: while (1 == 1)	{
	eval {
		$status = &libcdbpl::dbiseof ($test);
	};
	if ($status == 1)	{
		goto DONE_OUTER1;
	}
	eval {
		$result = &libcdbpl::dbretrieve ($test);
	};
	if ($result == 0)	{
		# skip deleted items
		$status = &libcdbpl::dbnextindex ($test);
		next LOOP1;
	}
	$name = $t_lname2 . $t_lname1;
	$t_lname1 -= 1;
	$lname = &libcdbpl::dbshow ($test, "lname");
	if ($lname ne $name)	{
		print ("lname ($lname) != name ($name)\n");
	}
	$fname = &libcdbpl::dbshow ($test, "fname");
	if ($fname ne $t_fname)	{
		print ("fname ($fname) != t_fname ($t_fname)\n");
	}
	$ssn = &libcdbpl::dbshow ($test, "ssn");
	if ($ssn ne $t_ssn) {
		print ("ssn ($ssn) != t_ssn ($t_ssn)\n");
	}
	$t_ssn += 1;
	&libcdbpl::dbnextindex ($test);
}
DONE_OUTER1:

print ("SSN index order done.\n");

&libcdbpl::dbcurrent ($test, "t2_midxname");
&libcdbpl::dbheadindex ($test);

$t_lname1 = 555555555;
$t_ssn = $t_lname1 + $iterations;

print ("Name index order...\n");
LOOP2: while (1 == 1)	{
	eval {
		$result = &libcdbpl::dbiseof ($test);
	};
	if ($result == 1) {
		goto DONE_OUTER2;
	}
	eval {
		$result = &libcdbpl::dbretrieve ($test);
	};
	if ($result == 0)	{
		# skip deleted items
		$status = &libcdbpl::dbnextindex ($test);
		next LOOP2;
	}
	$t_lname1 += 1;
	$name = $t_lname2 . $t_lname1;
	$lname = &libcdbpl::dbshow ($test, "lname");
	if ($lname ne $name) {
		print ("lname ($lname) != name ($name)\n");
	}
	$fname = &libcdbpl::dbshow ($test, "fname");
	if ($fname ne $t_fname)	{
		print ("fname ($fname) != t_fname ($t_fname)\n");
	}
	$ssn = &libcdbpl::dbshow ($test, "ssn");
	$t_ssn -= 1;
	if ($ssn ne $t_ssn)	{
		print ("ssn ($ssn) != t_ssn ($t_ssn)\n");
	}
	&libcdbpl::dbnextindex ($test);
}
DONE_OUTER2:

print ("Name index order done.\n");

$clock2 = &libcdbpl::dbtime;
$thisTime = $clock2-$clock1;
printf ("Finished evalutating table data in %6.4f seconds\n", $thisTime);

print ("Finished successfully.\n\n");

&libcdbpl::dbexit;
exit (0);

