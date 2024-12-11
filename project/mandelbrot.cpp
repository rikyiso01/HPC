#include <iostream>
#include <fstream>
#include <complex>
#include <chrono>
#include <omp.h>

#ifdef __NVCC__
#include <cuda.h>
#include <cuda/std/complex>
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
#define RESOLUTION 1000
#endif
#define WIDTH (RATIO_X * RESOLUTION)
#define HEIGHT (RATIO_Y * RESOLUTION)

#define STEP ((double)RATIO_X / WIDTH)

#ifndef DEGREE
#define DEGREE 2        // Degree of the polynomial
#endif
#ifndef ITERATIONS
#define ITERATIONS 1000 // Maximum number of iterations
#endif

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 16
#endif

using namespace std;

#ifdef __NVCC__

__global__ void kernel(int* image)
{
  int pos = threadIdx.x + blockIdx.x * blockDim.x;

  if(pos > WIDTH*HEIGHT) return;

  // evaluate derivatives

  image[pos] = 0;

  const int row = pos / WIDTH;
  const int col = pos % WIDTH;
  const cuda::std::complex<double> c(col * STEP + MIN_X, row * STEP + MIN_Y);

  // z = z^2 + c
  cuda::std::complex<double> z(0, 0);
  for (int i = 1; i <= ITERATIONS; i++)
  {
      z = cuda::std::pow(z, DEGREE) + c;

      // If it is convergent
      if (cuda::std::abs(z) >= 2)
      {
          image[pos] = i;
          break;
      }
  }

}

#endif

int main(int argc, char **argv)
{
    int *const image = new int[HEIGHT * WIDTH];

    // const auto start = chrono::steady_clock::now();
    const auto start = omp_get_wtime();

    #ifdef __NVCC__

    const int size=WIDTH*HEIGHT;

    int *image_dev;

    cudaMalloc((void**) &image_dev, size);

    cudaMemcpy(image_dev, image, size, cudaMemcpyHostToDevice);

    dim3 block_size(BLOCK_SIZE);
    dim3 grid_size = dim3((size - 1) / BLOCK_SIZE + 1);

    // Execute the modified version using same data
    kernel<<<grid_size, block_size>>>(image_dev);
    cudaMemcpy(image, image_dev, size, cudaMemcpyDeviceToHost);

    cudaDeviceSynchronize();

    #else

    #ifdef OMP
    # pragma omp parallel for default(none) shared(image)
    #endif
    for (int pos = 0; pos < HEIGHT * WIDTH; pos++)
    {
        image[pos] = 0;

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
                image[pos] = i;
                break;
            }
        }
    }

    #endif
    // const auto end = chrono::steady_clock::now();
    const auto end = omp_get_wtime();
    // cout << "Time elapsed: "
    //      << chrono::duration_cast<chrono::seconds>(end - start).count()
    //      << " seconds." << endl;
    cout << "Time elapsed: " << (end-start) << " seconds." << endl;

    // Write the result to a file
    ofstream matrix_out;

    if (argc < 2)
    {
        cout << "Please specify the output file as a parameter." << endl;
        return -1;
    }

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

    delete[] image; // It's here for coding style, but useless
    return 0;
}
