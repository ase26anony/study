#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int stride, 
                      volatile int n, volatile int m) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int firstprivate_val = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) firstprivate(firstprivate_val) \
        private(i, j) shared(n, m) num_teams(4) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end && (idx - start) % stride == 0) {
                int local_idx = (idx - start) / stride;
                c[local_idx] = a[local_idx] * b[local_idx] + firstprivate_val;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, int low, int high,
                          volatile float scale, volatile int offset) {
    float local_scale = scale;
    int local_offset = offset;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high], y[low:high]) map(tofrom: z[low:high]) \
        firstprivate(local_scale, local_offset) num_teams(8)
    for (int i = low; i < high; i++) {
        z[i] = x[i] * local_scale + y[i] + local_offset;
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *p, double *q, double *r, int size,
                         volatile int mode, volatile double factor) {
    const int block = 256;
    static double static_array[256];
    
    #pragma omp target data map(to: p[0:size], q[0:size]) map(from: r[0:size])
    {
        if (mode % 2 == 0) {
            #pragma omp target teams distribute parallel for simd \
                map(alloc: static_array[0:block]) \
                firstprivate(factor) collapse(2) simdlen(8)
            for (int i = 0; i < size; i += block) {
                for (int j = 0; j < block && (i + j) < size; j++) {
                    int idx = i + j;
                    static_array[j] = p[idx] * factor;
                    r[idx] = static_array[j] + q[idx];
                }
            }
        } else {
            #pragma omp target teams distribute parallel for \
                map(alloc: static_array[0:block]) \
                firstprivate(factor) num_teams(16)
            for (int i = 0; i < size; i += block) {
                for (int j = 0; j < block && (i + j) < size; j++) {
                    int idx = i + j;
                    static_array[j] = p[idx] / factor;
                    r[idx] = static_array[j] - q[idx];
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int seed) {
    int local_seed = seed;
    #pragma omp parallel for simd firstprivate(local_seed)
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * local_seed) % 1000;
    }
}

/* Function that conditionally calls target or host regions */
void conditional_execution(int *a, int *b, int *c, int size,
                           volatile int choice, volatile int bound) {
    if (choice % 3 == 0) {
        // Call SIMD target variant
        simd_target_loop(a, b, c, 0, size, 2, size/2, size/4);
    } else if (choice % 3 == 1) {
        // Call parallel target variant
        float *fa = (float*)a;
        float *fb = (float*)b;
        float *fc = (float*)c;
        parallel_target_loop(fa, fb, fc, 0, size/4, 2.5f, bound);
    } else {
        // Call host-only variant
        host_only_parallel(a, size, bound);
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for seed if provided
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    printf("Seed: %d\n", seed);
    
    // Declare arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    const int const_size = M;
    volatile int vol_bound = N;
    
    // Initialize arrays with random data
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
    }
    
    // Additional arrays for different types
    float float_array[N];
    double double_array[N];
    for (int i = 0; i < N; i++) {
        float_array[i] = (float)(rand() % 100) / 10.0f;
        double_array[i] = (double)(rand() % 100) / 10.0;
    }
    
    // Result arrays
    int result1[N], result2[N];
    float result_float[N];
    double result_double[N];
    
    // Clear result arrays
    memset(result1, 0, sizeof(result1));
    memset(result2, 0, sizeof(result2));
    memset(result_float, 0, sizeof(result_float));
    memset(result_double, 0, sizeof(result_double));
    
    long total_checksum = 0;
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int mode = rand() % 10;
        volatile int choice = rand() % 5;
        volatile int bound = 100 + rand() % 900;
        volatile float scale = 1.0f + (rand() % 100) / 50.0f;
        volatile double factor = 0.5 + (rand() % 100) / 200.0;
        
        printf("\nIteration %d: mode=%d, choice=%d\n", iter, mode, choice);
        
        // Call different variants based on conditions
        if (mode < 4) {
            // SIMD target loop with complex bounds
            int start = rand() % (N/2);
            int end = start + 100 + rand() % 200;
            int stride = 1 + rand() % 3;
            
            simd_target_loop(static_array, auto_array, result1, 
                           start, end, stride, vol_bound, const_size);
            
            // Calculate checksum
            int checksum = 0;
            for (int i = 0; i < N; i++) {
                checksum += result1[i];
            }
            total_checksum += checksum;
            printf("  SIMD target checksum: %d\n", checksum);
            
        } else if (mode < 7) {
            // Parallel target loop
            int low = rand() % (N/2);
            int high = low + 200 + rand() % 300;
            
            parallel_target_loop(float_array, (float*)auto_array, 
                               result_float, low, high, scale, bound);
            
            // Calculate checksum
            float fchecksum = 0.0f;
            for (int i = 0; i < N; i++) {
                fchecksum += result_float[i];
            }
            total_checksum += (long)fchecksum;
            printf("  Parallel target checksum: %.2f\n", fchecksum);
            
        } else {
            // Combined constructs
            int size = 200 + rand() % 300;
            
            combined_constructs(double_array, (double*)static_array,
                              result_double, size, choice, factor);
            
            // Calculate checksum
            double dchecksum = 0.0;
            for (int i = 0; i < size; i++) {
                dchecksum += result_double[i];
            }
            total_checksum += (long)dchecksum;
            printf("  Combined constructs checksum: %.2f\n", dchecksum);
        }
        
        // Conditional execution path
        conditional_execution(static_array, auto_array, result2, 
                            N/2, choice, bound);
    }
    
    printf("\nTotal checksum: %ld\n", total_checksum);
    
    // Final verification
    int final_check = 0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < N; i++) {
        final_check += static_array[i] + auto_array[i] + result1[i] + result2[i];
    }
    
    printf("Final verification: %d\n", final_check);
    
    return 0;
}
