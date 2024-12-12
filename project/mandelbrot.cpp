#include <iostream>
#include <fstream>
#include <complex>
#include <chrono>
#include <omp.h>
#ifdef MPI
#include <mpi.h>
#endif

// Ranges of the set
#ifndef MIN_X
#define MIN_X -2
#endif
#ifndef MAX_X
#define MAX_X 1
#endif
#ifndef MIN_Y
#define MIN_Y -1
#endif
#ifndef MAX_Y
#define MAX_Y 1
#endif

// Image ratio
#define RATIO_X (MAX_X - MIN_X)
#define RATIO_Y (MAX_Y - MIN_Y)

// Image size
#ifndef RESOLUTION
#define RESOLUTION 2000
#endif
#define WIDTH (RATIO_X * RESOLUTION)
#define HEIGHT (RATIO_Y * RESOLUTION)

#define STEP ((double)RATIO_X / WIDTH)

#ifndef DEGREE
#define DEGREE 2        // Degree of the polynomial
#endif
#ifndef ITERATIONS
#define ITERATIONS 10 // Maximum number of iterations
#endif

using namespace std;

// returns the array with all work assignments
inline int* splitWorkEqually(const int id, const int processes, const int totalWorkSize, int *workStart, int *workSize) {

    int avgWork = totalWorkSize / processes;
    int remainder = totalWorkSize % processes;

    int *assignments = new int[processes];

    for (int j = 0; j < remainder; ++j) assignments[j] = avgWork + 1;
    
    for (int j = remainder; j < processes; ++j) assignments[j] = avgWork;

    *workStart = id * avgWork + min(id, remainder);
    *workSize = assignments[id];

    return assignments;
}

// yes I need a prefix sum for MPI don't bother about it
inline int* getPrefixSum(const int *array, int arraySize) {

    int *prefixSum = new int[arraySize];
    prefixSum[0] = 0;

    for(int j = 1; j < arraySize; ++j) {
        prefixSum[j] = prefixSum[j-1] + array[j-1];
    }

    return prefixSum;
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        cout << "Please specify the output file as a parameter." << endl;
        return -1;
    }

    int id = 0;
    int workStart = 0;
    int workSize = HEIGHT * WIDTH;
    
    int* image = nullptr;
    int* imageBuffer = nullptr;

    // const auto start = chrono::steady_clock::now();
    double start;

    #ifdef MPI

        int commSize;
        int* assignments;
        int* displacements;

        MPI_Init(&argc, &argv);

        MPI_Comm_rank(MPI_COMM_WORLD, &id);
        MPI_Comm_size(MPI_COMM_WORLD, &commSize);

    #endif

    if (id == 0) {
        start = omp_get_wtime();
        image = new int[HEIGHT * WIDTH];
        imageBuffer = image; // this is important so that we can use the same code both with and without MPI
    }

    #ifdef MPI

        // assignments and displacements are only needed by root process, but since they do not occupy 
        // much space I will free them only after the computation has completed
        assignments = splitWorkEqually(id, commSize, HEIGHT * WIDTH, &workStart, &workSize);
        displacements = getPrefixSum(assignments, commSize);

        imageBuffer = new int[workSize];

    #endif
            

    #ifdef OMP
    # pragma omp parallel for default(none) shared(image)
    #endif
    for (int j = 0; j < workSize; j++)
    {
        imageBuffer[j] = 0;

        const int pos = j + workStart;
        const int row = pos / WIDTH;
        const int col = pos % WIDTH;
        const complex<double> c(col * STEP + MIN_X, row * STEP + MIN_Y);

        // z = z^2 + c
        complex<double> z(0, 0);
        for (int i = 1; i <= ITERATIONS; i++)
        {
            z = pow(z, DEGREE) + c;

            // If it is convergent
            if (abs(z) >= 2)
            {
                imageBuffer[j] = i;
                break;
            }
        }
    }

    #ifdef MPI

    MPI_Gatherv(imageBuffer, workSize, MPI_INT, image, assignments, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    #endif

    if (id == 0) {
        const double end = omp_get_wtime();

        cout << "Time elapsed: " << (end-start) << " seconds." << endl;

        // Write the result to a file
        ofstream matrix_out;

        matrix_out.open(argv[1], ios::trunc);
        if (!matrix_out.is_open())
        {
            cout << "Unable to open file." << endl;
            return -2;
        }

        for (int row = 0; row < HEIGHT; row++)
        {
            for (int col = 0; col < WIDTH; col++)
            {
                matrix_out << image[row * WIDTH + col];

                if (col < WIDTH - 1)
                    matrix_out << ',';
            }
            if (row < HEIGHT - 1)
                matrix_out << endl;
        }
        matrix_out.close();

    }

    delete[] image; // It's here for coding style, but useless
    #ifdef MPI
        delete[] assignments;
        delete[] displacements;
        delete[] imageBuffer;
    #endif
    
    return 0;
}