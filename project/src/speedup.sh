source ./common.sh

title "OMP speedup"

# compile "$SPEEDUP" -g -xHost -O3 -ffast-math -DOMP
# roofline omp-roofline
# measure "omp-roofline"


rm -rf "$OUT/speedup"

for i in 1 2 4 12 20 30
do
    compile "$SPEEDUP" -g -xHost -O3 -ffast-math -DOMP
    echo "OMP_NUM_THREADS=$i" >> "$OUT/speedup"
    time OMP_NUM_THREADS=$i $COMPILED_FILE "$PROGRAM_OUTPUT" | tee -a "$OUT/speedup"
done

