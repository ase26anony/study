#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int * restrict a, int * restrict b, int * restrict c, 
                      int start, int end, int stride, int n) {
    static int static_counter = 0;
    const int chunk_size = 64;
    volatile int vol_bound = end;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        firstprivate(static_counter) private(chunk_size) \
        collapse(2) num_teams(4) thread_limit(128)
    for (int i = start; i < vol_bound; i += stride) {
        for (int j = 0; j < chunk_size; j++) {
            int idx = i * chunk_size + j;
            if (idx < n) {
                c[idx] = a[idx] * static_counter + b[idx] / (j + 1);
                static_counter = (static_counter + 1) % 100;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float * restrict x, float * restrict y, 
                          float * restrict z, int low, int high, 
                          int offset, int m) {
    const float scale = 2.5f;
    volatile int vol_high = high;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high], y[low:high]) \
        map(from: z[low:high]) \
        firstprivate(scale, offset) shared(m) \
        num_teams(8)
    for (int i = low; i < vol_high; i++) {
        float temp = 0.0f;
        for (int k = 0; k < offset; k++) {
            temp += x[i] * y[(i + k) % m] * scale;
        }
        z[i] = temp / offset;
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double * restrict p, double * restrict q,
                         double * restrict r, int size, int block) {
    static double static_array[256];
    volatile int vol_size = size;
    
    /* Initialize static array with some values */
    #pragma omp parallel for simd
    for (int i = 0; i < 256; i++) {
        static_array[i] = i * 0.1;
    }
    
    #pragma omp target data map(to: p[0:vol_size], q[0:vol_size]) \
                            map(from: r[0:vol_size])
    {
        #pragma omp target teams distribute parallel for simd \
            map(alloc: static_array[0:256]) \
            firstprivate(block) reduction(+:static_array[0:256]) \
            collapse(2)
        for (int i = 0; i < vol_size; i += block) {
            for (int j = 0; j < block && (i + j) < vol_size; j++) {
                int idx = i + j;
                r[idx] = p[idx] * q[idx] + static_array[idx % 256];
                static_array[idx % 256] += 0.01;
            }
        }
    }
}

/* Variant 4: Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int n, int factor) {
    volatile int vol_n = n;
    
    #pragma omp parallel for simd
    for (int i = 0; i < vol_n; i++) {
        arr[i] = arr[i] * factor + i;
    }
}

/* Helper function to compute checksum */
long long compute_checksum_int(int *arr, int n) {
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_checksum_float(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

double compute_checksum_double(double *arr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Allocate and initialize arrays with different storage durations */
    int *a = (int *)malloc(N * M * sizeof(int));
    int *b = (int *)malloc(N * M * sizeof(int));
    int *c = (int *)malloc(N * M * sizeof(int));
    
    float *x = (float *)malloc(N * sizeof(float));
    float *y = (float *)malloc(N * sizeof(float));
    float *z = (float *)malloc(N * sizeof(float));
    
    double *p = (double *)malloc(N * sizeof(double));
    double *q = (double *)malloc(N * sizeof(double));
    double *r = (double *)malloc(N * sizeof(double));
    
    /* Initialize with random data */
    #pragma omp parallel for simd
    for (int i = 0; i < N * M; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = 0;
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        x[i] = (float)rand() / RAND_MAX;
        y[i] = (float)rand() / RAND_MAX;
        z[i] = 0.0f;
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        p[i] = (double)rand() / RAND_MAX;
        q[i] = (double)rand() / RAND_MAX;
        r[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMT transformation test with seed %d\n", seed);
    
    /* Main test loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int use_target = (rand() % 3) > 0;  /* Mostly use target regions */
        int start = rand() % 100;
        int end = N - rand() % 100;
        int stride = 1 + rand() % 3;
        int offset = 1 + rand() % 10;
        int block = 16 + rand() % 32;
        
        printf("\nIteration %d: start=%d, end=%d, stride=%d, offset=%d, block=%d\n",
               iter, start, end, stride, offset, block);
        
        if (use_target) {
            /* Call target region variants */
            if (iter % 2 == 0) {
                simd_target_loop(a, b, c, start, end, stride, N * M);
                long long checksum = compute_checksum_int(c, N * M);
                printf("  SIMD target loop checksum: %lld\n", checksum);
            } else {
                parallel_target_loop(x, y, z, start, end, offset, N);
                float checksum = compute_checksum_float(z, N);
                printf("  Parallel target loop checksum: %f\n", checksum);
            }
            
            if (iter % 3 == 0) {
                combined_constructs(p, q, r, N, block);
                double checksum = compute_checksum_double(r, N);
                printf("  Combined constructs checksum: %f\n", checksum);
            }
        } else {
            /* Call host-only variant occasionally */
            host_only_parallel(a, N * M, iter + 1);
            long long checksum = compute_checksum_int(a, N * M);
            printf("  Host-only parallel checksum: %lld\n", checksum);
        }
        
        /* Shuffle data between iterations to create dependencies */
        #pragma omp parallel for simd
        for (int i = 0; i < N; i++) {
            x[i] = y[i] * 0.5f + z[i];
            p[i] = q[i] * 0.7 + r[i];
        }
    }
    
    /* Final verification */
    printf("\nFinal verification:\n");
    printf("Array c checksum: %lld\n", compute_checksum_int(c, N * M));
    printf("Array z checksum: %f\n", compute_checksum_float(z, N));
    printf("Array r checksum: %f\n", compute_checksum_double(r, N));
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(p); free(q); free(r);
    
    return 0;
}
