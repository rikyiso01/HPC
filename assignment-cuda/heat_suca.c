#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef __NVCC__
#include <cuda.h>
#endif
#include <sys/time.h>

// Simple define to index into a 1D array from 2D space
#define I2D(num, c, r) ((r)*(num)+(c))

/*
 * `step_kernel_mod` is currently a direct copy of the CPU reference solution
 * `step_kernel_ref` below. Accelerate it to run as a CUDA kernel.
 */

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 16
#endif
#define BLOCK_WIDTH BLOCK_SIZE
#define BLOCK_HEIGHT BLOCK_SIZE
#ifndef SIZE
#define SIZE 1000
#endif
#ifndef STEPS
#define STEPS 200
#endif

#ifdef __NVCC__
#ifndef SHARED
#ifdef PITCH
__global__ void step_kernel_mod(int ni, int nj, float fact, float* temp_in, float* temp_out, size_t pitch1, size_t pitch2)
#else
__global__ void step_kernel_mod(int ni, int nj, float fact, float* temp_in, float* temp_out)
#endif
{

  /*
    COMMENTS: Remember about coalescence (cudaMallocPitch())
    remeber balancing between L1 and Shared Mem cudaDeviceSetCacheConfig
    __shared__ to access shared memory
    __constant__ to set something on constant memory
  */
  //int i00, im10, ip10, i0m1, i0p1;
  float d2tdx2, d2tdy2;

  int x = threadIdx.x + blockIdx.x * blockDim.x + 1;
  int y = threadIdx.y + blockIdx.y * blockDim.y + 1;

  if(x >= ni-1 || y >= nj-1) return;

  // evaluate derivatives

  #ifndef PITCH
  d2tdx2 = temp_in[I2D(ni, x - 1, y)] - 2 * temp_in[I2D(ni, x, y)] + temp_in[I2D(ni, x + 1, y)];
  d2tdy2 = temp_in[I2D(ni, x, y - 1)] - 2 * temp_in[I2D(ni, x, y)] + temp_in[I2D(ni, x, y + 1)];

  // update temperatures
  temp_out[I2D(ni, x, y)] = temp_in[I2D(ni, x, y)]+ fact * (d2tdx2 + d2tdy2);
  #else
  d2tdx2 = temp_in[I2D(pitch1, x - 1, y)] - 2 * temp_in[I2D(pitch1, x, y)] + temp_in[I2D(pitch1, x + 1, y)];
  d2tdy2 = temp_in[I2D(pitch1, x, y - 1)] - 2 * temp_in[I2D(pitch1, x, y)] + temp_in[I2D(pitch1, x, y + 1)];

  // update temperatures
  temp_out[I2D(pitch2, x, y)] = temp_in[I2D(pitch1, x, y)] + fact * (d2tdx2 + d2tdy2);
  #endif
}
#else
__global__ void step_kernel_mod(int ni, int nj, float fact, float* temp_in, float* temp_out)
{

  /*
    COMMENTS: Remember about coalescence (cudaMallocPitch())
    remeber balancing between L1 and Shared Mem cudaDeviceSetCacheConfig
    __shared__ to access shared memory
    __constant__ to set something on constant memory
  */
  //int i00, im10, ip10, i0m1, i0p1;
  float d2tdx2, d2tdy2;

  __shared__ float sharedMem[(BLOCK_WIDTH + 2) * (BLOCK_HEIGHT + 2)];

  int x = threadIdx.x + blockIdx.x * blockDim.x + 1;
  int y = threadIdx.y + blockIdx.y * blockDim.y + 1;

  if(x >= ni-1 || y >= nj-1) return;

  sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y + 1)] = temp_in[I2D(ni, x, y)];

// TODO: put most of this inside the same warp

  if(threadIdx.x == 0) sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x, threadIdx.y + 1)] = temp_in[I2D(ni, x - 1, y)];
  else if(threadIdx.x == blockDim.x - 1 || x == ni - 2) sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 2, threadIdx.y + 1)] = temp_in[I2D(ni, x + 1, y)];

  if(threadIdx.y == 0) sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y)] = temp_in[I2D(ni, x, y - 1)];
  else if(threadIdx.y == blockDim.y - 1 || y == nj - 2) sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y + 2)] = temp_in[I2D(ni, x, y + 1)];

  __syncthreads();

  // evaluate derivatives
  d2tdx2 = sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x, threadIdx.y + 1)]
    - 2 * sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y + 1)]
    + sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 2, threadIdx.y + 1)];

  d2tdy2 = sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y)]
    - 2 * sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y + 1)]
    + sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y + 2)];

    //d2tdx2 = temp_in[I2D(ni, x - 1, y)] - 2 * temp_in[I2D(ni, x, y)] + temp_in[I2D(ni, x + 1, y)];
    //d2tdy2 = temp_in[I2D(ni, x, y - 1)] - 2 * temp_in[I2D(ni, x, y)] + temp_in[I2D(ni, x, y + 1)];

  // update temperatures
  temp_out[I2D(ni, x, y)] = sharedMem[I2D(BLOCK_WIDTH + 2, threadIdx.x + 1, threadIdx.y + 1)] + fact * (d2tdx2 + d2tdy2);

}
#endif
#endif

void step_kernel_ref(int ni, int nj, float fact, float* temp_in, float* temp_out)
{
  int i00, im10, ip10, i0m1, i0p1;
  float d2tdx2, d2tdy2;


  // loop over all points in domain (except boundary)
  for ( int j=1; j < nj-1; j++ ) {
    for ( int i=1; i < ni-1; i++ ) {
      // find indices into linear memory
      // for central point and neighbours
      i00 = I2D(ni, i, j);
      im10 = I2D(ni, i-1, j);
      ip10 = I2D(ni, i+1, j);
      i0m1 = I2D(ni, i, j-1);
      i0p1 = I2D(ni, i, j+1);

      // evaluate derivatives
      d2tdx2 = temp_in[im10]-2*temp_in[i00]+temp_in[ip10];
      d2tdy2 = temp_in[i0m1]-2*temp_in[i00]+temp_in[i0p1];

      // update temperatures
      temp_out[i00] = temp_in[i00]+fact*(d2tdx2 + d2tdy2);
    }
  }
}

int main()
{
  struct timeval stop, start;
  int istep;
  int nstep = STEPS; // number of time steps

  // Specify our 2D dimensions
  const int ni = SIZE;
  const int nj = SIZE;
  float tfac = 8.418e-5; // thermal diffusivity of silver

  float *temp1_ref, *temp2_ref, *temp1, *temp2, *temp_tmp;

  const int size = ni * nj * sizeof(float);

  temp1_ref = (float*)malloc(size);
  temp2_ref = (float*)malloc(size);
  temp1 = (float*)malloc(size);
  temp2 = (float*)malloc(size);

  // Initialize with random data
  for( int i = 0; i < ni*nj; ++i) {
    temp1_ref[i] = temp2_ref[i] = temp1[i] = temp2[i] = (float)rand()/(float)(RAND_MAX/100.0f);
  }

  gettimeofday(&start, NULL);

#ifndef __NVCC__

  // Execute the CPU-only reference version
  for (istep=0; istep < nstep; istep++) {
    step_kernel_ref(ni, nj, tfac, temp1_ref, temp2_ref);

    // swap the temperature pointers
    temp_tmp = temp1_ref;
    temp1_ref = temp2_ref;
    temp2_ref= temp_tmp;
  }

#else

  float *temp1_dev, *temp2_dev;

#ifdef PITCH
  size_t pitch1;
  cudaMallocPitch((void**) &temp1_dev, &pitch1,nj,ni);
  size_t pitch2;
  cudaMallocPitch((void**) &temp2_dev, &pitch2,nj,ni);

  cudaMemcpy2D(temp1_dev, pitch1,temp1, nj,nj,ni, cudaMemcpyHostToDevice);
  cudaMemcpy2D(temp2_dev, pitch2,temp2, nj,nj,ni, cudaMemcpyHostToDevice);

  dim3 block_size(BLOCK_WIDTH, BLOCK_HEIGHT);
  dim3 grid_size = dim3((nj - 3) / BLOCK_WIDTH + 1, (ni - 3) / BLOCK_HEIGHT + 1);

  size_t pitch_tmp;
  // Execute the modified version using same data
  for (istep=0; istep < nstep; istep++) {
    step_kernel_mod<<<grid_size, block_size>>>(ni, nj, tfac, temp1_dev, temp2_dev, pitch1, pitch2);

    // swap the temperature pointers
    temp_tmp = temp1_dev;
    temp1_dev = temp2_dev;
    temp2_dev = temp_tmp;

    pitch_tmp = pitch1;
    pitch1 = pitch2;
    pitch2 = pitch_tmp;
  }
  cudaMemcpy(temp1, temp1_dev, size, cudaMemcpyDeviceToHost);

  cudaDeviceSynchronize();

#else
  cudaMalloc((void**) &temp1_dev, size);
  cudaMalloc((void**) &temp2_dev, size);

  cudaMemcpy(temp1_dev, temp1, size, cudaMemcpyHostToDevice);
  cudaMemcpy(temp2_dev, temp2, size, cudaMemcpyHostToDevice);

  dim3 block_size(BLOCK_WIDTH, BLOCK_HEIGHT);
  dim3 grid_size = dim3((nj - 3) / BLOCK_WIDTH + 1, (ni - 3) / BLOCK_HEIGHT + 1);

  // Execute the modified version using same data
  for (istep=0; istep < nstep; istep++) {
    step_kernel_mod<<<grid_size, block_size>>>(ni, nj, tfac, temp1_dev, temp2_dev);

    // swap the temperature pointers
    temp_tmp = temp1_dev;
    temp1_dev = temp2_dev;
    temp2_dev = temp_tmp;
  }
  cudaMemcpy(temp1, temp1_dev, size, cudaMemcpyDeviceToHost);

  cudaDeviceSynchronize();
#endif

#endif

  gettimeofday(&stop, NULL);
  printf("took %lf s\n", ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec)/1000000.0);

  // float maxError = 0;
  // // Output should always be stored in the temp1 and temp1_ref at this point
  // for( int i = 0; i < ni*nj; ++i ) {
  //   if (abs(temp1[i]-temp1_ref[i]) > maxError) { maxError = abs(temp1[i]-temp1_ref[i]); }
  // }
  //
  // // Check and see if our maxError is greater than an error bound
  // if (maxError > 0.0005f)
  //   printf("Problem! The Max Error of %.5f is NOT within acceptable bounds.\\n", maxError);
  // else
  //   printf("The Max Error of %.5f is within acceptable bounds.\\n", maxError);

  free( temp1_ref );
  free( temp2_ref );
  free( temp1 );
  free( temp2 );

#ifdef __NVCC__
  cudaFree(tmp1_dev);
  cudaFree(tmp2_dev);
#endif

  return 0;
}

