#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step) {
    volatile int v_start = start;  /* Prevent constant folding */
    volatile int v_end = end;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[v_start:v_end], b[v_start:v_end]) \
        map(from: c[v_start:v_end]) \
        private(v_start, v_end) \
        firstprivate(step) \
        collapse(2) \
        num_teams(4) \
        thread_limit(128)
    for (int i = v_start; i < v_end; i += step) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            if (idx < N * M) {
                c[idx] = a[idx] + b[idx] * step;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD */
void parallel_target_loop(float *x, float *y, float *z, int low, int high, int stride) {
    static const float scale = 2.5f;  /* Mix static with automatic */
    volatile int v_low = low;
    volatile int v_high = high;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[v_low:v_high:v_stride], y[v_low:v_high:v_stride]) \
        map(from: z[v_low:v_high:v_stride]) \
        shared(scale) \
        firstprivate(stride) \
        num_teams(8)
    for (int i = v_low; i < v_high; i += stride) {
        float temp = 0.0f;
        for (int j = 0; j < 16; j++) {
            int idx = i + j;
            if (idx < N) {
                temp += x[idx] * y[idx];
            }
        }
        z[i] = temp * scale / stride;
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *p, double *q, double *r, int size, int offset) {
    volatile int v_size = size;
    const int chunk = 64;  /* Mix const with volatile */
    int *dynamic_array = (int*)malloc(v_size * sizeof(int));
    
    /* Initialize dynamic array with pattern */
    for (int i = 0; i < v_size; i++) {
        dynamic_array[i] = i % 32;
    }
    
    #pragma omp target data map(to: p[0:v_size], q[0:v_size], dynamic_array[0:v_size]) \
                            map(from: r[0:v_size])
    {
        #pragma omp target teams distribute parallel for simd \
            map(alloc: dynamic_array[0:v_size]) \
            firstprivate(offset, chunk) \
            collapse(2) \
            thread_limit(256)
        for (int i = 0; i < v_size; i += chunk) {
            for (int j = 0; j < chunk; j++) {
                int idx = i + j;
                if (idx < v_size) {
                    double factor = (double)dynamic_array[idx] / 32.0;
                    r[idx] = p[idx] * factor + q[idx] * (1.0 - factor) + offset;
                }
            }
        }
    }
    
    free(dynamic_array);
}

/* Variant 4: Host-only parallel region (for conditional execution) */
void host_only_parallel(int *arr1, int *arr2, int n) {
    volatile int v_n = n;
    #pragma omp parallel for simd
    for (int i = 0; i < v_n; i++) {
        arr1[i] = arr2[i] * 2 + i;
    }
}

/* Function to select between target and host execution */
void conditional_execution(int *a, int *b, int *c, int size, int selector) {
    if (selector % 3 == 0) {
        /* Call target SIMD variant */
        simd_target_loop(a, b, c, 0, size, 2);
    } else if (selector % 3 == 1) {
        /* Call host-only variant */
        host_only_parallel(a, b, size);
        memcpy(c, a, size * sizeof(int));
    } else {
        /* Call parallel target variant */
        parallel_target_loop((float*)a, (float*)b, (float*)c, 
                           0, size/4, 4);
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for random seed */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Declare arrays with different storage durations */
    static int static_array[N * M];          /* Static storage */
    int auto_array[N * M];                   /* Automatic storage */
    float float_array[N];
    double double_array[N];
    
    /* Initialize arrays with random/sequential data */
    for (int i = 0; i < N * M; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = i % 100;
    }
    
    for (int i = 0; i < N; i++) {
        float_array[i] = (float)rand() / RAND_MAX * 10.0f;
        double_array[i] = (double)rand() / RAND_MAX * 20.0;
    }
    
    int result_array[N * M];
    float result_float[N];
    double result_double[N];
    
    long total_checksum = 0;
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int iter_mod = iter;  /* Prevent optimization */
        
        /* Vary parameters based on iteration and random seed */
        int start_idx = (rand() % 100);
        int end_idx = N * M - (rand() % 100);
        int step = 1 + (iter % 4);
        int offset = rand() % 50;
        
        /* Call different variants based on iteration */
        switch (iter % 4) {
            case 0:
                simd_target_loop(static_array, auto_array, result_array,
                               start_idx, end_idx, step);
                break;
            case 1:
                parallel_target_loop(float_array, float_array + N/2,
                                   result_float, 0, N/2, 2 + iter);
                break;
            case 2:
                combined_constructs(double_array, double_array + N/2,
                                  result_double, N/2, offset);
                break;
            case 3:
                conditional_execution(static_array, auto_array, result_array,
                                    N * M / 2, iter + seed);
                break;
        }
        
        /* Compute checksum to prevent dead code elimination */
        long iter_checksum = 0;
        for (int i = 0; i < N * M && i < 100; i++) {
            iter_checksum += result_array[i];
        }
        for (int i = 0; i < N && i < 50; i++) {
            iter_checksum += (long)result_float[i];
            iter_checksum += (long)result_double[i];
        }
        
        total_checksum += iter_checksum;
        printf("Iteration %d checksum: %ld\n", iter, iter_checksum);
    }
    
    printf("Total checksum: %ld\n", total_checksum);
    printf("Random seed used: %d\n", seed);
    
    return 0;
}
