X=ffmpeg
PD=$1

test -d "$PD" || exit 1

cd "$PD" || exit 1

echo Source $PD tmp $X

j=1

for i in $(find . -type f -name '*.jpg' | sort | head -500) ; do
	f=$(printf "$X%015d.jpg" $j)
	echo $j $i $f
	ln -sf $i $f
	j=$(($j + 1))
done

# -c:v libx264
ffmpeg -framerate 25 -i $X%015d.jpg -profile:v high -crf 20 -pix_fmt yuv420p output.mp4

#ffmpeg -framerate 25 -i $X%015d.jpg  -profile:v high -crf 20 output.swf
#mv output.swf ../

#ffmpeg -framerate 25 -i $X%015d.jpg output.mpg
#mv output.mpg ../

exit 0

