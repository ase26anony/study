#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int bound1, volatile int bound2) {
    #pragma omp target teams distribute parallel for simd \
                map(to: a[start:end], b[start:end]) \
                map(from: c[start:end]) \
                collapse(2) \
                private(bound1, bound2) \
                firstprivate(start, end, step)
    for (int i = 0; i < bound1; i++) {
        for (int j = 0; j < bound2; j++) {
            int idx = start + (i * bound2 + j) * step;
            if (idx < end) {
                c[idx] = a[idx] * 2 + b[idx] / 3;
                /* Complex indexing to prevent optimization */
                c[idx] += (i % 4) * (j % 3);
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD */
void parallel_target_loop(float *x, float *y, float *z, 
                          int low, int high, int stride,
                          volatile int dyn_bound) {
    const float scale = 2.5f;
    static float local_scale = 1.8f;
    
    #pragma omp target data map(to: x[low:high:stride], y[low:high:stride]) \
                            map(from: z[low:high:stride])
    {
        #pragma omp target teams distribute parallel for \
                    firstprivate(scale, local_scale) \
                    shared(x, y, z) \
                    private(dyn_bound)
        for (int i = 0; i < dyn_bound; i++) {
            int idx = low + i * stride;
            if (idx < high) {
                /* Mixed operations to create complex data flow */
                z[idx] = x[idx] * scale + y[idx] / local_scale;
                z[idx] += (i % 8) * 0.125f;
                local_scale = scale * 0.9f; /* Modified in parallel region */
            }
        }
    }
}

/* Variant 3: Combined constructs with pointer arithmetic */
void combined_constructs(double *p, double *q, double *r,
                         int *offsets, int num_offsets,
                         volatile int outer_bound) {
    double *local_p = p;
    double *local_q = q;
    double *local_r = r;
    
    #pragma omp target data map(to: local_p[0:N], local_q[0:N]) \
                            map(from: local_r[0:N]) \
                            map(to: offsets[0:num_offsets])
    {
        #pragma omp target teams distribute parallel for simd \
                    collapse(2) \
                    firstprivate(num_offsets) \
                    shared(offsets)
        for (int block = 0; block < outer_bound; block++) {
            for (int elem = 0; elem < 16; elem++) {
                int idx = (block * 16 + elem) % N;
                if (idx < N && block < num_offsets) {
                    /* Complex pointer arithmetic */
                    double *src1 = local_p + idx + offsets[block % num_offsets];
                    double *src2 = local_q + idx - offsets[block % num_offsets];
                    double *dst = local_r + idx;
                    
                    if (src1 >= local_p && src1 < local_p + N &&
                        src2 >= local_q && src2 < local_q + N) {
                        *dst = *src1 * 1.5 + *src2 * 0.5;
                        *dst += (block % 4) * 0.25;
                    }
                }
            }
        }
    }
}

/* Variant 4: Host-only parallel region for comparison */
void host_only_parallel(int *arr1, int *arr2, int size, volatile int limit) {
    #pragma omp parallel for simd
    for (int i = 0; i < limit; i++) {
        if (i < size) {
            arr1[i] = arr2[i] * 3 - i;
        }
    }
}

/* Helper to initialize arrays with pattern */
void init_array(void *arr, size_t size, int seed, int type) {
    if (type == 0) { /* int */
        int *iarr = (int *)arr;
        for (size_t i = 0; i < size / sizeof(int); i++) {
            iarr[i] = (seed + i * 7) % 100;
        }
    } else if (type == 1) { /* float */
        float *farr = (float *)arr;
        for (size_t i = 0; i < size / sizeof(float); i++) {
            farr[i] = (seed + i) * 0.123f;
        }
    } else { /* double */
        double *darr = (double *)arr;
        for (size_t i = 0; i < size / sizeof(double); i++) {
            darr[i] = (seed + i) * 0.456;
        }
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Mixed storage durations and types */
    static int static_arr[N];
    int auto_arr[N];
    float float_arr[M];
    double double_arr[N];
    int offsets[8];
    
    /* Initialize with patterns */
    init_array(static_arr, sizeof(static_arr), seed, 0);
    init_array(auto_arr, sizeof(auto_arr), seed * 2, 0);
    init_array(float_arr, sizeof(float_arr), seed * 3, 1);
    init_array(double_arr, sizeof(double_arr), seed * 4, 2);
    
    for (int i = 0; i < 8; i++) {
        offsets[i] = (rand() % 20) - 10;
    }
    
    /* Result arrays */
    int result_int[N] = {0};
    float result_float[M] = {0.0f};
    double result_double[N] = {0.0};
    
    printf("Starting OpenMP SIMT transformation test with seed %d\n", seed);
    
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int bound1 = (rand() % 64) + 32;
        volatile int bound2 = (rand() % 32) + 16;
        volatile int dyn_bound = (rand() % 128) + 64;
        volatile int outer_bound = (rand() % 24) + 8;
        
        int start = rand() % (N/2);
        int end = start + (rand() % (N/4)) + 32;
        int step = (rand() % 3) + 1;
        
        int low = rand() % (M/2);
        int high = low + (rand() % (M/4)) + 64;
        int stride = (rand() % 2) + 1;
        
        /* Randomly choose between target and host execution paths */
        if (rand() % 2) {
            printf("Iteration %d: Using SIMD target loop\n", iter);
            simd_target_loop(static_arr, auto_arr, result_int, 
                           start, end, step, bound1, bound2);
        } else {
            printf("Iteration %d: Using host-only parallel\n", iter);
            host_only_parallel(result_int, static_arr, N, dyn_bound);
        }
        
        /* Always call the parallel target variant */
        printf("Iteration %d: Using parallel target loop\n", iter);
        parallel_target_loop(float_arr, float_arr, result_float,
                           low, high, stride, dyn_bound);
        
        /* Call combined constructs with probability */
        if (rand() % 3 != 0) {
            printf("Iteration %d: Using combined constructs\n", iter);
            combined_constructs(double_arr, double_arr, result_double,
                              offsets, 8, outer_bound);
        }
        
        /* Compute checksums to prevent dead code elimination */
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum, float_sum, double_sum)
        for (int i = 0; i < N; i++) {
            if (i < N) int_sum += result_int[i];
            if (i < N) double_sum += result_double[i];
            if (i < M) float_sum += result_float[i];
        }
        
        printf("Iteration %d checksums: int=%d, float=%.2f, double=%.2f\n",
               iter, int_sum, float_sum, double_sum);
        
        /* Modify inputs for next iteration */
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            static_arr[i] += iter;
            auto_arr[i] -= iter;
            if (i < M) float_arr[i] *= 1.01f;
            double_arr[i] /= 1.001;
        }
    }
    
    printf("Test completed successfully\n");
    return 0;
}
