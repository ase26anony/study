#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512

/* Function variants with different OpenMP constructs */

/* Uses target teams distribute parallel for simd */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int vsize = end - start; /* volatile to prevent constant folding */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        private(start) firstprivate(step) shared(vsize)
    for (int i = start; i < end; i += step) {
        int idx = i;
        if (idx < end) {
            c[idx] = a[idx] + b[idx] * (vsize % 256);
        }
    }
}

/* Uses target teams distribute parallel for (no simd clause) */
void parallel_target_loop(float *x, float *y, float *z, int low, int high, int stride) {
    static const int chunk = 64; /* mixed storage duration */
    volatile int vlimit = high;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:stride], y[low:high:stride]) \
        map(from: z[low:high:stride]) \
        firstprivate(chunk, stride) shared(vlimit)
    for (int i = low; i < vlimit; i += stride) {
        float sum = 0.0f;
        for (int j = 0; j < chunk; j++) { /* nested loop */
            sum += x[i] * y[i] * j;
        }
        z[i] = sum;
    }
}

/* Combined constructs with target data region */
void combined_constructs(double *p, double *q, double *r, int n, int m) {
    int *temp = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) temp[i] = i % 100;
    
    #pragma omp target data map(to: p[0:n], q[0:n], temp[0:n]) map(from: r[0:n])
    {
        /* Complex iteration space with collapse */
        #pragma omp target teams distribute parallel for simd collapse(2) \
            private(m) firstprivate(n)
        for (int i = 0; i < n; i += 2) {
            for (int j = 0; j < m; j++) {
                int idx = i + j;
                if (idx < n) {
                    r[idx] = p[idx] * q[idx] + temp[idx] * (j + 1);
                }
            }
        }
    }
    free(temp);
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 2 + 1;
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) seed = atoi(argv[1]);
    srand(seed);
    
    /* Arrays with different types and storage */
    static int arr1[N]; /* static storage */
    int arr2[N];        /* automatic storage */
    float arr3[M];
    double arr4[N];
    double arr5[N];
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        if (i < M) arr3[i] = (float)(rand() % 1000) / 10.0f;
        arr4[i] = (double)(rand() % 1000) / 20.0;
        arr5[i] = (double)(rand() % 1000) / 20.0;
    }
    
    int checksum = 0;
    
    /* Loop with varying parameters to expose multiple contexts */
    for (int iter = 0; iter < 5; iter++) {
        volatile int viter = iter; /* volatile to prevent optimization */
        int start = rand() % (N/4);
        int end = N - rand() % (N/4);
        int step = 1 + (rand() % 3);
        int low = rand() % (M/2);
        int high = M - rand() % (M/4);
        int stride = 1 + (rand() % 2);
        
        /* Conditional execution based on random input */
        if (rand() % 2) {
            printf("Iteration %d: Calling simd_target_loop\n", iter);
            simd_target_loop(arr1, arr2, arr2, start, end, step);
        } else {
            printf("Iteration %d: Calling host_only_parallel\n", iter);
            host_only_parallel(arr1, N);
        }
        
        /* Always call these to ensure SIMT transformation contexts */
        parallel_target_loop(arr3, arr3, arr3, low, high, stride);
        combined_constructs(arr4, arr5, arr4, N, 8 + (rand() % 16));
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < N; i += 32) {
            checksum += arr1[i] + arr2[i] + (int)arr4[i];
        }
        for (int i = 0; i < M; i += 16) {
            checksum += (int)arr3[i];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
