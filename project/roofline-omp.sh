source ./common.sh

title "OMP parallel roofline"

compile "$OMP_ROOFLINE" -xHost -O3 -ffast-math -DOMP
roofline omp-roofline
measure "omp-roofline"
