source ./common.sh

title Roofline

compile "$ROOFLINE" -g
roofline
snapshot "hotspot"
measure hotspot

compile "$ROOFLINE" -g -qopt-report=3 -xHost -O3
mv ./mandelbrot.optrpt $OUT
measure opt-report

compile "$ROOFLINE" -g
roofline
snapshot "hotspot-best-sequential"
measure hotspot-best-sequential
