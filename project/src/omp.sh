source ./common.sh

title "OMP parallel"

compile "$OMP" -xHost -O3 -ffast-math
measure omp-sequential

compile "$OMP" -xHost -O3 -ffast-math -DOMP
measure omp-parallel
