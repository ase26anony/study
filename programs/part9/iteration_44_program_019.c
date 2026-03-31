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
                      int start, int end, int stride, int n, volatile int flag) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int local_private = flag;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) firstprivate(local_private) \
        private(static_counter) num_teams(end-start) thread_limit(256)
    for (int i = start; i < end; i += stride) {
        for (int j = 0; j < chunk_size; j++) {
            int idx = i * chunk_size + j;
            if (idx < n) {
                // Complex indexing with pointer arithmetic
                int *ptr_a = a + idx;
                int *ptr_b = b + idx;
                int *ptr_c = c + idx;
                *ptr_c = *ptr_a * local_private + *ptr_b * (j % 8);
                static_counter++;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float * restrict x, float * restrict y, 
                          float * restrict z, int low, int high, 
                          volatile int bound) {
    const float scale = 2.5f;
    float firstprivate_val = (float)bound;
    
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high], y[low:high]) map(from: z[low:high]) \
        firstprivate(firstprivate_val, scale) shared(bound) \
        num_teams((high-low+63)/64) thread_limit(128)
    for (int i = low; i < high; i++) {
        // Runtime-dependent branching
        if (bound > 0) {
            z[i] = x[i] * scale + y[i] * firstprivate_val;
        } else {
            z[i] = x[i] / scale - y[i] * firstprivate_val;
        }
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double * restrict d1, double * restrict d2,
                         double * restrict d3, int size, volatile int mode) {
    static double static_array[256];
    const double pi = 3.141592653589793;
    
    // Initialize static array
    #pragma omp parallel for simd
    for (int i = 0; i < 256; i++) {
        static_array[i] = (double)i * pi;
    }
    
    #pragma omp target data map(to: d1[0:size], d2[0:size]) \
                            map(from: d3[0:size]) map(tofrom: static_array)
    {
        int teams = (size + 255) / 256;
        if (teams > 32) teams = 32;
        
        #pragma omp target teams distribute parallel for simd \
            collapse(2) firstprivate(mode, pi) shared(static_array) \
            num_teams(teams) thread_limit(64)
        for (int i = 0; i < size; i += 16) {
            for (int j = 0; j < 16; j++) {
                int idx = i + j;
                if (idx < size) {
                    // Complex computation with conditional SIMD lanes
                    double temp = d1[idx] * d2[idx];
                    if (mode & 1) {
                        d3[idx] = temp + static_array[idx % 256] * pi;
                    } else {
                        d3[idx] = temp - static_array[idx % 256] / pi;
                    }
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int n, volatile int seed) {
    int local_seed = seed;
    
    #pragma omp parallel for simd schedule(dynamic, 16)
    for (int i = 0; i < n; i++) {
        arr[i] = (arr[i] * local_seed + i) % 1000;
    }
}

/* Function to select between target and host execution */
void conditional_execution(int *a, int *b, int *c, int n, 
                           volatile int selector, volatile int bound) {
    if (selector > 0) {
        // Call target region function
        simd_target_loop(a, b, c, 0, n/2, 2, n, bound);
    } else {
        // Call host-only function
        host_only_parallel(a, n, bound);
        host_only_parallel(b, n, bound);
        #pragma omp parallel for simd
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for random seed
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage classes
    static int static_array[N];
    int auto_array[N];
    volatile int volatile_bound = rand() % 100 + 1;
    
    float float_array[M];
    double double_array[N];
    
    // Initialize arrays with mixed patterns
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        static_array[i] = i * 3;
        auto_array[i] = i * 2 + 1;
        double_array[i] = (double)i / 10.0;
    }
    
    #pragma omp parallel for
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)i * 1.5f;
    }
    
    // Result arrays
    int result_int[N];
    float result_float[M];
    double result_double[N];
    
    // Clear result arrays
    memset(result_int, 0, sizeof(result_int));
    memset(result_float, 0, sizeof(result_float));
    memset(result_double, 0, sizeof(result_double));
    
    // Main execution loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int mode = rand() % 4;
        volatile int selector = rand() % 3;
        volatile int bound = rand() % 200 + 50;
        
        printf("Iteration %d: mode=%d, selector=%d, bound=%d\n", 
               iter, mode, selector, bound);
        
        // Call different variants based on mode
        switch (mode) {
            case 0:
                simd_target_loop(static_array, auto_array, result_int,
                                 iter * 64, (iter + 2) * 64, 1 + iter % 3,
                                 N, bound);
                break;
            case 1:
                parallel_target_loop(float_array, float_array + M/2,
                                     result_float, iter * 32, 
                                     (iter + 4) * 32, bound);
                break;
            case 2:
                combined_constructs(double_array, double_array + N/2,
                                    result_double, N - iter * 128, selector);
                break;
            case 3:
                conditional_execution(static_array, auto_array, result_int,
                                      N - iter * 64, selector, bound);
                break;
        }
        
        // Compute checksums to prevent dead code elimination
        long long int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        #pragma omp parallel for reduction(+:int_sum, float_sum, double_sum)
        for (int i = 0; i < N; i++) {
            if (i < N) int_sum += result_int[i];
            if (i < N) double_sum += result_double[i];
            if (i < M) float_sum += result_float[i];
        }
        
        printf("  Checksums: int=%lld, float=%f, double=%f\n",
               int_sum, float_sum, double_sum);
        
        // Modify input arrays for next iteration
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            static_array[i] = (static_array[i] + iter) % 1000;
            auto_array[i] = (auto_array[i] * 3 + 1) % 1000;
            double_array[i] += 0.1 * iter;
        }
        
        #pragma omp parallel for
        for (int i = 0; i < M; i++) {
            float_array[i] += 0.5f * iter;
        }
    }
    
    // Final verification with complex reduction
    int final_check = 0;
    #pragma omp target teams distribute parallel for reduction(+:final_check) \
        map(to: result_int[0:N]) map(tofrom: final_check)
    for (int i = 0; i < N; i++) {
        final_check += result_int[i] % 256;
    }
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
