source ./common.sh

title Roofline

compile "$ROOFLINE1" "$ROOFLINE2" -g
roofline "roofline-normal"

compile "$ROOFLINE1" "$ROOFLINE2" -g -qopt-report=3 -xHost -O3
mv ./heat.optrpt $OUT
measure opt-report

# compile "$ROOFLINE" -g -xHost -O3 -ffast-math
# roofline "roofline-optimized"
#
compile "$ROOFLINE1" "$ROOFLINE2" -xHost -O3 -ffast-math
measure roofline-optimized
