source ./common.sh

title "Sequential"

# compile "$SEQUENTIAL"
# measure sequential-normal

compile "$SEQUENTIAL1" "$SEQUENTIAL2" -xHost -O3 -ffast-math
measure sequential-optimized

