source ./common.sh

title "OMP parallel roofline"

compile "$OMP_ROOFLINE" -g -xHost -O3 -ffast-math -DOMP
roofline roofline-omp-roofline
