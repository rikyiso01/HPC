source ./common.sh

title "Sequential"

compile -O0 "$SEQUENTIAL"
measure sequential-normal

compile "$SEQUENTIAL" -xHost -O3 -ffast-math
measure sequential-optimized

