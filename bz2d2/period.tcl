# $Id: period.tcl,v 1.2 2003/11/30 01:23:10 petterik Exp $

set data {}



proc ac {dat} {
	set N [llength $dat]
	puts "$N samples"

	for {set k 0} {$k != $N} {incr k} {
		set sum 0.0
		set jend [expr {$N - $k - 1}]
		for {set j 0} {$j != $jend} {incr j} {
			set i [expr {$j + $k}]
			set sum [expr {$sum + ([lindex $dat $j] * [lindex $dat $j])}]
			#puts "$j $ac1 $i $ac2"; #exit 1 			
		}
		lappend C [expr {$sum/($N - $k)}]
	}
   	set N [llength $C]
	puts "$N samples"
	return $C
}

proc std {dat} {
	set N [llength $dat]

	set avg 0.0
	foreach i $dat {
		set avg [expr {$avg + $i}]
	}	
	set avg [expr {$avg/$N}]
	set s 0.0
	foreach i $dat {
		set d [expr {$avg  - $i}]
		set s [expr {$s + $d * $d}]
	}
	set s [expr {sqrt($s/($N - 1))}]
	# average, standard deviation, standard error
	return [list $avg $s [expr {$s/sqrt($N)}]] 
}

proc cc {x y} {
	set C {}
	set N [llength $x]

	set mx [lindex [std $x] 0]
	set my [lindex [std $y] 0]

	set sx 0.0
	set sy 0.0
	for {set i 0} {$i != $N} {incr i} {
		set xi [expr {[lindex $x $i] - $mx}]
		set yi [expr {[lindex $y $i] - $my}]
		set sx [expr {$sx + $xi * $xi}]
		set sy [expr {$sy + $yi * $yi}]
	}
	set denom [expr {$sx * $sy}]

	for {set k [expr -$N/2]} {$k != [expr $N/2]} {incr k} {
		set sxy 0.0
		for {set i 0} {$i != $N} {incr i} {
			set j [expr {$i + $k}]
			if {$j > 0 && $j < $N} {
				set sxy [expr {$sxy + ([lindex $x $i] - $mx)*([lindex $y $i] - $my)}]
			}
		}
		lappend C [expr {$sxy/$denom}] 
	}
	return $C
}

proc period {dat} {
	set N [llength $dat]
	#puts "$N samples"

	set lt 0.0
	set go 0 ;# first seen
	set down 1
	set dtsum 0.0
	set dtcnt 0
	set dtlst {}
	foreach {t x} $dat {
		#puts $t
		set dt [expr {$t - $lt}]
		if {$x > 0.03 &&  $dt > 1.0} {
			set lt $t			
			if {$go == 0} {
				set go 1
			} elseif {$down == 1} {
				set dtsum [expr {$dtsum + $dt}]
				lappend dtlst $dt
				incr dtcnt
				set down 0
			}
		} elseif {$x < 0.01} {
			set down 1
		}
	}
	if {$dtcnt == 0} {
		puts stderr "warning: zero samples"
		set dtsum 0.0
	} else {
		#puts "$dtcnt samples"
		set dtsum [expr {$dtsum/$dtcnt}]
	}

	return [std $dtlst]
}

set A {}
set B {}
set i 0
set fp [open bz2d2.dat r]
while {[gets $fp line] >= 0 && [incr i] <= 512000} {
	if {[scan $line %f%f%f t a b] == 3} {
		lappend A $t $a
		lappend B $t $b
	}
}
close $fp

set avals {}
set bvals {}
foreach {t v} $A {
	lappend avals $v
}

foreach {t v} $B {
	lappend bvals $v
}

foreach i [cc $avals $bvals] {
	puts $i
}

exit 0

set ares [period $A]
set aavg [lindex $ares 0]
set astd [lindex $ares 1]
set bres [period $B]
set bavg [lindex $bres 0]
set bstd [lindex $bres 1]
set delta [expr {$astd/$aavg + $bstd/$bavg}]
puts "[expr {$bavg/$aavg}] $delta"

exit 0
