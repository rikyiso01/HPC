source ./common.sh

title "Best sequential"

compile "$BEST_SEQUENTIAL" -xHost -O3
measure best-sequential-normal

compile "$BEST_SEQUENTIAL" -xHost -O3 -ffast-math
measure best-sequential-fast-math

compile "$BEST_SEQUENTIAL" -xHost -O3 -ffast-math -ipo
measure best-sequential-fast-math-ipo

compile "$BEST_SEQUENTIAL" -xHost -O3 -ipo
measure best-sequential-ipo
