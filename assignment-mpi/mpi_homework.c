#include <stdio.h>
#include <time.h>
#include <mpi.h>
               
#define PI25DT 3.141592653589793238462643

#define INTERVALS 55000000000

int main(int argc, char **argv)
{
    long int i, intervals = INTERVALS;
    double x, dx, f, sum, pi, result;
    double time2;
    
    time_t time1 = clock();

    printf("Number of intervals: %ld\n", intervals);

    sum = 0.0;
    dx = 1.0 / (double) intervals;

    int rank;
    int size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int work = intervals / size;
    int remainder = intervals % size;

    if (rank < remainder) { // then increase this process personal work by 1 and set its starting point to its rank * work_of_previous_processes
        work++;
        i = rank * work;
    } else {
        i = rank * work + remainder; // == (remainder * (work + 1)) + (rank - remainder) * work; 
    }

    int limit = i + work;

    for (; i < limit; i++) {
        // changed this to + 0.5 because it's simpler having i start from 0
        x = dx * ((double) (i + 0.5));
        f = 4.0 / (1.0 + x*x);
        sum = sum + f;
    }

    MPI_Reduce(&result, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        pi = dx*result;

        time2 = (clock() - time1) / (double) CLOCKS_PER_SEC;

        printf("Computed PI %.24f\n", pi);
        printf("The true PI %.24f\n\n", PI25DT);
        printf("Elapsed time (s) = %.2lf\n", time2);
    }

    MPI_Finalize();
         
    

    return 0;
}