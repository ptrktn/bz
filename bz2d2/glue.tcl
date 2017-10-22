
set i 0

while {[file exists chaos/$i.gif] && [file exists nochaos/$i.gif]} {
	exec montage -mode concatenate -tile 2x1 chaos/$i.gif nochaos/$i.gif GIF:$i.gif
	incr i
}

