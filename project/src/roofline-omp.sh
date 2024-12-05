source ./common.sh

title "OMP parallel roofline"

compile "$OMP_ROOFLINE" -g -xHost -O3 -ffast-math -DOMP
roofline omp-roofline
snapshot "omp-roofline"
measure "omp-roofline"
