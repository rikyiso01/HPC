source ./common.sh

title Roofline

compile "$ROOFLINE" -g
roofline "roofline-normal"

compile "$ROOFLINE" -g -qopt-report=3 -xHost -O3
mv ./mandelbrot.optrpt $OUT
measure opt-report

compile "$ROOFLINE" -g -xHost -O3 -ffast-math
roofline "roofline-optimized"
