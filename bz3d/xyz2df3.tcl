# $Id: xyz2df3.tcl,v 1.7 2003/11/09 09:05:38 petterik Exp $
# convert tip data into df3 density files


proc dat2inc {} {
	puts "union \{"
	
	while {[gets stdin line] >= 0} {
		# incoming data from stdin is: z x y
		if {[scan $line %d%f%f z x y] == 3} {
			puts "  sphere \{ <$x,$y,$z>, 1.0 texture \{bbox\} \}"
			puts "  sphere \{ <$x,$y,0>, 0.75 texture \{pigment \{rgb <1,1,1>\}\} \}"
		}
	}

	puts "  translate -CC\n\}"
}

if {[lsearch $argv --dat2inc] >= 0} {
	dat2inc
	exit 0
}

if {[llength $argv] != 2} {
	puts "usage: <input data> <output df3>"
	exit 1
}


proc intVals {mz x0 y0 z0 x1 y1 z1} {
	set res {}
	set nz [expr {$mz * ($z1 - $z0)}]
	set dx [expr {($x1 - $x0)/($nz)}]
	set dy [expr {($y1 - $y0)/($nz)}]

	for {set z 0} {$z < $nz} {incr z} {
		set ix [expr {$x0 + $z * $dx}]
		set iy [expr {$y0 + $z * $dy}]
		set iz [expr {$z + $mz * $z0}]
		lappend res $ix $iy $iz
	}

	return $res
}

proc intTip {arr last steps} {
	set res {}
	upvar $arr dat
	set nz [llength [array names dat]]

	# assume dat(0) always exists
	set i 0
	set j 1

	while {$i < $last} {
		while {![info exists dat($j)]} {
			incr j
		}
		foreach val [intVals $steps \
						 [lindex $dat($i) 0] [lindex $dat($i) 1] $i \
						 [lindex $dat($j) 0] [lindex $dat($j) 1] $j] {
			lappend res $val
		}
		set i $j
		incr j
	}
	return $res
}

proc convDat {dat nx ny} {
	set res {}

	set fp [open tip.inc w]
	puts $fp "union \{"

	set nz [expr {[llength $dat]/30}]
	set c 0

	foreach {x y z} $dat {
		if {![expr {$z % 10}]} {
			set tip($c) [list $x $y]
			incr c
		}
	}

	set nz [llength [array names tip]]

	puts "$nx $ny $nz"

	append res [binary format c 0]
	append res [binary format c $nx]
	append res [binary format c 0]
	append res [binary format c $ny]
	append res [binary format c 0]
	append res [binary format c $nz]
	set max 0
	for {set z 0} {$z < $nz} {incr z} {
		set x0 [lindex $tip($z) 0] 
		set y0 [lindex $tip($z) 1]
		puts $fp "  sphere \{ <$x0,$y0,$z>, 1.0 texture \{bbox\} \}"

		for {set y 0} {$y < $ny} {incr y} {
			for {set x 0} {$x < $nx} {incr x} {
				set x0 [lindex $tip($z) 0] 
				set y0 [lindex $tip($z) 1]
				set dx [expr {abs($x - $x0)}]
				set dy [expr {abs($y - $y0)}]
				set d [expr {sqrt($dx * $dx + $dy * $dy)}]
				if {$d < 1.5} {
					scan [expr {255.0/(1.0 + $d)}] \
						%d.%d vox dummy
					if {$max < $vox} {
						set max $vox
					}
				} else {
					set vox 0
				}
				append res [binary format c $vox]
			}
		}
	}
	puts "max=$max"

	puts $fp "  translate -CC\n\}"
	close $fp

	return $res
}

set ifp [open [lindex $argv 0] r]
set ofp [open [lindex $argv 1] w]
fconfigure $ofp -translation binary

while {[gets $ifp line] >= 0} {
	if {[scan $line %d%f%f z x y] == 3} {
		set x [expr {$x * 1.0}]
		set y [expr {$y * 1.0}]
		set tip($z) [list $x $y] 
	}
}

set nz 96
set nx 32
set ny 32

set int [intTip tip 95 10]
set int [convDat $int $nx $ny]

puts -nonewline $ofp $int

close $ifp
close $ofp

exit 0
