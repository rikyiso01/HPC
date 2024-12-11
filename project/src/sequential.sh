source ./common.sh

title "Sequential"

compile "$SEQUENTIAL" -O3
measure sequential-normal

compile "$SEQUENTIAL" -xHost -O3 -ffast-math
measure sequential-optimized

