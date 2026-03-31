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
                      volatile int n, volatile int m) {
    #pragma omp target teams distribute parallel for simd \
                map(to: a[start:end], b[start:end]) \
                map(from: c[start:end]) \
                collapse(2) if(n > 100)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end) {
                c[idx] = a[idx] + b[idx] * (i + 1);
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, int stride,
                         volatile int limit) {
    const float scale = 2.5f;
    static float offset = 1.0f;
    
    #pragma omp target teams distribute parallel for \
                map(to: x[low:high:stride], y[low:high:stride]) \
                map(from: z[low:high:stride]) \
                private(scale) firstprivate(offset) \
                if(limit > 50)
    for (int i = 0; i < limit; i++) {
        int idx = low + i * stride;
        if (idx < high) {
            z[idx] = x[idx] * scale + y[idx] + offset;
            /* Complex indexing to prevent optimization */
            z[idx] += (i % 3 == 0) ? x[(idx + 1) % high] : y[(idx - 1 + high) % high];
        }
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *p, double *q, double *r,
                        int size, volatile int iter) {
    double local_sum = 0.0;
    const double alpha = 1.5;
    
    #pragma omp target data map(to: p[0:size], q[0:size]) \
                            map(from: r[0:size]) \
                            map(tofrom: local_sum)
    {
        #pragma omp target teams distribute parallel for simd \
                    reduction(+:local_sum) \
                    if(iter % 2 == 0)
        for (int i = 0; i < size; i++) {
            r[i] = alpha * p[i] + q[i] / (i + 1);
            /* Pointer arithmetic */
            double *ptr = &r[i];
            *ptr += (i > 0) ? r[i-1] * 0.1 : 0.0;
            local_sum += r[i];
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile double dummy = local_sum;
    (void)dummy;
}

/* Variant 4: Nested function with conditional target */
void nested_target_region(int *arr1, int *arr2, int dim1, int dim2,
                         volatile int threshold) {
    int local_arr[100];
    for (int i = 0; i < 100; i++) local_arr[i] = i;
    
    /* Runtime condition that may affect SIMT transformation */
    if (threshold > 100) {
        #pragma omp target teams distribute parallel for simd \
                    map(to: arr1[0:dim1*dim2]) \
                    map(tofrom: arr2[0:dim1*dim2]) \
                    collapse(2)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                arr2[idx] = arr1[idx] * local_arr[idx % 100] + (i * j);
            }
        }
    } else {
        /* Host-only parallel region for comparison */
        #pragma omp parallel for simd
        for (int i = 0; i < dim1 * dim2; i++) {
            arr2[i] = arr1[i] + 1;
        }
    }
}

/* Helper to initialize arrays */
void init_arrays(int *a, int *b, int *c, float *x, float *y, float *z,
                double *p, double *q, double *r, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
        c[i] = 0;
        x[i] = i * 0.5f;
        y[i] = i * 1.5f;
        z[i] = 0.0f;
        p[i] = i * 0.25;
        q[i] = i * 0.75;
        r[i] = 0.0;
    }
}

/* Checksum verification */
int verify_results(int *c, float *z, double *r, int size) {
    int checksum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    for (int i = 0; i < size; i++) {
        checksum += c[i];
        fsum += z[i];
        dsum += r[i];
    }
    
    checksum += (int)fsum + (int)dsum;
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Mixed storage duration arrays */
    static int static_array[N * M];
    int auto_array1[N * M], auto_array2[N * M];
    float float_array1[N * M], float_array2[N * M], float_array3[N * M];
    double double_array1[N * M], double_array2[N * M], double_array3[N * M];
    
    /* Initialize all arrays */
    init_arrays(auto_array1, auto_array2, static_array,
                float_array1, float_array2, float_array3,
                double_array1, double_array2, double_array3,
                N * M);
    
    int total_checksum = 0;
    
    /* Multiple iterations with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        /* Volatile variables to prevent constant folding */
        volatile int v_n = N + (rand() % 100) - 50;
        volatile int v_m = M + (rand() % 50) - 25;
        volatile int limit = 200 + (rand() % 300);
        volatile int threshold = rand() % 200;
        
        /* Varying array slices */
        int start = (iter * 100) % (N * M);
        int end = start + 500;
        int stride = 1 + (iter % 3);
        
        printf("Iteration %d: n=%d, m=%d, threshold=%d\n", 
               iter, v_n, v_m, threshold);
        
        /* Call variant functions with different patterns */
        if (iter % 3 == 0) {
            simd_target_loop(auto_array1, auto_array2, static_array,
                           start, end, stride, v_n, v_m);
        } else if (iter % 3 == 1) {
            parallel_target_loop(float_array1, float_array2, float_array3,
                               start, end, stride, limit);
        } else {
            combined_constructs(double_array1, double_array2, double_array3,
                              N * M, iter);
        }
        
        /* Always call nested version for conditional SIMT path */
        nested_target_region(auto_array1, auto_array2, v_n, v_m, threshold);
        
        /* Verify and accumulate checksum */
        int iter_checksum = verify_results(static_array, float_array3, 
                                         double_array3, N * M);
        total_checksum += iter_checksum;
        
        printf("  Checksum: %d\n", iter_checksum);
    }
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Use results to prevent optimization */
    volatile int final_result = total_checksum;
    return final_result % 100;
}
