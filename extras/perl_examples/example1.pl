use libcdbpl;

#
# First, create the definition file.
#
if ( -f "testpl.df" ) {
	system ("rm -f testpl.df");
}

open (TESTPL_DF, ">testpl.df") || die "***Error: testpl.df cannot be opened\n";

print TESTPL_DF '// testpl.df', "\n";
print TESTPL_DF 'create table "t1_test1.db"', "\n";
print TESTPL_DF '  info "Test table for testing Perl bindings"', "\n";
print TESTPL_DF '{', "\n";
print TESTPL_DF '  "ssn" char (9);', "\n";
print TESTPL_DF '  "lname" char (30);', "\n";
print TESTPL_DF '  "fname" char (15);', "\n";
print TESTPL_DF '  "salary" number (10:2);' , "\n";
print TESTPL_DF '  "dob" date;', "\n";
print TESTPL_DF '  "entered" time;', "\n";
print TESTPL_DF '  "married" logical; ', "\n";
print TESTPL_DF '  "numpid" number (3:0);', "\n";
print TESTPL_DF '  "pid" char (8);', "\n";
print TESTPL_DF '} indexed { ', "\n";
print TESTPL_DF '  idx "t1_tpid" 256:case:unique "pid";', "\n";
print TESTPL_DF '  idx "t1_idxssn" 256:case:unique "ssn";', "\n";
print TESTPL_DF '  midx "t1_midxname" 512:nocase "lname", "fname";', "\n";
print TESTPL_DF '};', "\n";
print TESTPL_DF 'create table "t1_test2.db"', "\n";
print TESTPL_DF '  info "Secondary test table for testing Tcl/Tk bindings"', "\n";
print TESTPL_DF '{', "\n";
print TESTPL_DF '  "pid" char (8);', "\n";
print TESTPL_DF '  "string" char (80);', "\n";
print TESTPL_DF '} indexed {', "\n";
print TESTPL_DF '  idx "t1_t2pid" 256:case:dup "pid";', "\n";
print TESTPL_DF '};', "\n";
close TESTPL_DF;

if ( -f "t1_test1.db" )	{
	system ("rm -f t1_test1.db t1_test1.db.LCK t1_tpid.idx t1_tpid.inx t1_idxssn.idx t1_idxssn.inx t1_midxname.idx t1_midxname.inx");
}

if ( -f "t1_test2.db" )	{
	system ("rm -f t1_test2.db t1_test2.db.LCK t1_t2pid.idx t1_t2pid.inx");
}

eval {
	$status = &libcdbpl::dbcreate ("testpl.df");
};
if ($@)	{
	print ("***Error: dbcreate: " . $@ . "\n");
	exit -1;
}

eval {
	$test = &libcdbpl::dbopen ("t1_test1.db");
};

if ($@)	{
	print ("***Error: dbopen: " . $@ . "\n");
	exit -1;
}

eval {
	$test2 = &libcdbpl::dbopen ("t1_test2.db");
};

if ($@)	{
	print ("***Error: dbopen: " . $@ ."\n");
	&libcdbpl::dbexit;
	exit 1;
}

eval {
	$this_seq = &libcdbpl::dbseq ($test);
};

if ($@)	{
	print ("***Error: dbseq: setting sequence numer of $test: " . $@ . "\n");
	&libcdbpl::dbexit;
	exit 1;
}

system ("rm -f testpl.df");

#
# Set some variables for testing.
#
$iterations = 25000;

$t_ssn1 = 999999990;
$t_ssn2 = 111111110;
$ssn_add = 2;
$ssn_sub = -3;
$ssnToAdd = 0;
$t_lname1 = 999;
$t_lname2 = "Test";
$t_fname = "Tcl";
$t_salary = 40000.00;
$t_dob1 = 1000;
$t_dob2 = 1028;
$t_married = 0;
$t_numpid = 1;
$t_numdir = 1;

$t2_string = "This is a test of the Perl bindings for DCDB.  What do you think?";

$clock1 = &libcdbpl::dbtime;

for ($i = 0; $i < $iterations; $i++)	{
	if ($ssnToAdd == 0)	{
		eval {
		&libcdbpl::dbsetchar ($test, "ssn", $t_ssn1);
		};
		if ($@)	{
			print ("***Error: dbsetchar: setting ssn in $test: " . $@ . "\n");
			&libcdbpl::dbexit;
			exit -1;
		}
		$ssnToAdd = 1;
		$t_ssn1 += $ssn_sub;
	}
	else	{
		eval {
		&libcdbpl::dbsetchar ($test, "ssn", $t_ssn2);
		};
		if ($@)	{
			print ("***Error: dbsetchar: setting ssn in $test: " . $@ . "\n");
			&libcdbpl::dbexit;
			exit -1;
		}
		$ssnToAdd = 0;
		$t_ssn2 += $ssn_add;
	}
	$name = $t_lname1;
	$t_lname1 -= 1;
	$name = $t_lname2 . $name;
	eval {
	&libcdbpl::dbsetchar ($test, "lname", $name);
	};
	if ($@)	{
		print ("***Error: dbsetchar: lname in $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}
	eval {
	&libcdbpl::dbsetchar ($test, "fname", $t_fname);
	};
	if ($@)	{
		printf ("***Error: dbsetchar: fname in $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}
	eval {
	&libcdbpl::dbsetnum ($test, "salary", $t_salary);
	};
	if ($@)	{
		printf ("***Error: dbsetnum: salary in $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}
	$t_salary += 235.50;
	$dateofbirth = $t_dob1;
	$t_dob1 += 1;
	if ($t_dob1 == 2000)	{
		$t_dob1 = 1000;
	}
	$dateofbirth = $dateofbirth . $t_dob2;
	eval {
	&libcdbpl::dbsetdate ($test, "dob", $dateofbirth);
	};
	if ($@)	{
		printf ("***Error: dbsetdate, salary in $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}
	eval {
	&libcdbpl::dbsettime ($test, "entered", 0);
	};
	if ($@) {
		printf ("***Error: dbsettime, entered in $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}
	if ($t_married == 0)	{
		$t_married = 1;
		eval {
		&libcdbpl::dbsetlog ($test, "married", "Y");
		};
		if ($@) {
			printf ("***Error: dbsetlog, married in $test: " . $@ . "\n");
			&libcdbpl::dbexit;
			exit -1;
		}
	}
	else	{
		$t_married = 0;
		eval {
		&libcdbpl::dbsetlog ($test, "married", "N");
		};
		if ($@) {
			printf ("***Error: dbsetlog, married in $test: " . $@ . "\n");
			&libcdbpl::dbexit;
			exit -1;
		}
	}
	if ($t_numpid == 1)	{
		$t_numpid = 1;
		$t_numdir = 1;
	}
	if ($t_numpid == 10)	{
		$t_numpid = 10;
		$t_numdir = 0;
	}
	if ($t_numdir == 0)	{
		$t_numpid -= 1;
	}
	if ($t_numdir == 1)	{
		$t_numpid += 1;
	}
	eval {
	&libcdbpl::dbsetint ($test, "numpid", $t_numpid);
	};
	if ($@)	{
		printf ("***Error: dbsetint, numpid in $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}

	eval {
	$this_seq = &libcdbpl::dbseq ($test);
	};
	if ($@)	{
		print ("***Error: dbseq: setting sequence on $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit 1;
	}
	$sequence = sprintf ("%s%07d", "T", $this_seq);
	eval {
	&libcdbpl::dbsetchar ($test, "pid", $sequence);
	};
	if ($@)	{
		print ("***Error: dbsetchar: pid on $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}
	eval {
	&libcdbpl::dbadd ($test);
	};
	if ($@)	{
		print ("***Error: dbadd on $test: " . $@ . "\n");
		&libcdbpl::dbexit;
		exit -1;
	}
	for ($j = 0; $j < $t_numpid; $j++)	{
		eval {
		&libcdbpl::dbsetchar ($test2, "pid", $sequence);
		};
		if ($@) {
			printf ("***Error: dbsetchar, pid on $test2: " . $@ . "\n");
			&libcdbpl::dbexit;
			exit -1;
		}
		eval {
		&libcdbpl::dbsetchar ($test2, "string", $t2_string);
		};
		if ($@) {
			printf ("***Error: dbsetchar, string on $test2: " . $@ . "\n");
			&libcdbpl::dbexit;
			exit -1;
		}
		eval {
		&libcdbpl::dbadd ($test2);
		};
		if ($@)	{
			printf ("***Error: dbadd on $test2, " . $@ . "\n");
			&libcdbpl::dbexit;
			exit -1;
		}
	}
}

$num_test = &libcdbpl::dbnumrecs ($test);
$num_test2 = &libcdbpl::dbnumrecs ($test2);

printf ("\n\nFinished: $num_test records in $test, $num_test2 records in $test2\n");
$clock2 = &libcdbpl::dbtime;
$thisTime = $clock2-$clock1;
printf ("Finished in %6.4f seconds\n\n", $thisTime);


eval {
&libcdbpl::dbcurrent ($test, "t1_idxssn");
};
eval {
&libcdbpl::dbheadindex ($test);
};
eval {
$status = &libcdbpl::dbiseof ($test);
};
if ($status == 1)	{
	printf ("There are no records in t1_test1.db!");
	&libcdbpl::dbexit;
	exit -1;
}

eval {
&libcdbpl::dbcurrent ($test2, "t1_t2pid");
};

$clock1 = &libcdbpl::dbtime;

LOOP1: while (1 == 1)	{
	eval {
	$result = &libcdbpl::dbiseof ($test);
	};
	if ($result == 1)	{
		goto DONE_OUTER;
	}
	eval {
	$result = &libcdbpl::dbretrieve ($test);
	};
	if ($result == 0)	{
		# skip deleted items
		$status = &libcdbpl::dbnextindex ($test);
		next LOOP1;
	}
	eval {
	$this_pid = &libcdbpl::dbshow ($test, "pid");
	};
	eval {
	$status = &libcdbpl::dbsearchexact ($test2, $this_pid);
	};
	if ($status != 0)	{
		$num_pid = 0;
		LOOP2: while (1 == 1)	{
			eval {
			$result = &libcdbpl::dbretrieve ($test2);
			};
			if ($result == 0)	{
				$status = &libcdbpl::dbnextindex ($test2);
				next LOOP2;
			}
			eval {
			$test2_pid = &libcdbpl::dbshow ($test2, "pid");
			};
			if ($test2_pid ne $this_pid)	{
				goto DONE_INNER;
			}
			$num_pid += 1;
			eval {
			&libcdbpl::dbnextindex ($test2);
			};
			eval {
			$result = &libcdbpl::dbiseof ($test2);
			};
			if ($result == 1)	{
				goto DONE_INNER;
			}
		}
	}

	DONE_INNER:

	eval {
	$this_numpid = &libcdbpl::dbshow ($test, "numpid");
	};
	if ($this_numpid != $num_pid)	{
		print ("***Warning: t1_test1.db->numpid != number of entries in t1_test2.db for item $this_pid\n");
	}
	eval {
	&libcdbpl::dbnextindex ($test);
	};
}
DONE_OUTER:

$clock2 = &libcdbpl::dbtime;
$thisTime = $clock2-$clock1;

printf ("\nFinished evaluating table data in %6.4f seconds\n", $thisTime);

printf ("\n\nFinished successfully\n\n");

&libcdbpl::dbexit;
exit 0;
