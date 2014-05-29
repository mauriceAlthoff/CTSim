#!/usr/bin/perl -Tw

my $mapfile = "../src/ctsim-map.h";
my $hhcfile = "ctsim.hhc";
my $newhhc = "ctsim-new.hhc";
my %map;

open (MAP, $mapfile) || print "Unable to open map file $mapfile";
while (<MAP>) {
    m|^\W*#define\W+IDH_(\w+)\W+(\w+)|;
    $map{$1} = $2 if ($1 && $2);
}
close (MAP);

open(HHC,$hhcfile) || print "Unable to open existing hhc file $hhcfile";
open(NEWHHC,"> $newhhc") || print "Unable to open new hhc file $newhhc";
while (<HHC>) {
    $line=$_;
    if ($line =~ m|(.*)\#IDH_([A-Za-z_]+)(.*)|) {
	my $varname=$2;
	print NEWHHC  $1 . $3 . "\n";
	if ($map{$2}) {
	    print NEWHHC "<param name=\"ID\" value=\"$map{$2}\">\n";
	} else {
	    print "Warning: unable to find IDH_$varname in $mapfile";
	}
    } else {
	print NEWHHC $line;
    }

}

close(HHC);
close(NEWHHC);

unlink($hhcfile);
rename($newhhc,$hhcfile);

exit(0);
