#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *restrict a, int *restrict b, int *restrict c, 
                      int start, int end, int stride, volatile int n) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int local_n = n;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) num_teams(8) thread_limit(128) \
        map(to: a[start:end:stride], b[start:end:stride], local_n) \
        map(from: c[start:end:stride]) \
        private(static_counter) firstprivate(chunk_size) \
        shared(a, b, c)
    for (int i = 0; i < local_n; i += chunk_size) {
        for (int j = 0; j < chunk_size && (i + j) < local_n; j++) {
            int idx = start + (i + j) * stride;
            c[idx] = a[idx] * 2 + b[idx] / 3;
            static_counter++;  /* Static variable access */
        }
    }
}

/* Variant 2: Target loop without SIMD clause */
void parallel_target_loop(float *restrict x, float *restrict y, 
                         float *restrict z, int low, int high,
                         volatile float scale) {
    float local_scale = scale;
    float *section_x = &x[low];
    float *section_y = &y[low];
    float *section_z = &z[low];
    int length = high - low;
    
    #pragma omp target data map(to: section_x[0:length], section_y[0:length]) \
                            map(from: section_z[0:length])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(4) thread_limit(64) \
            firstprivate(local_scale, length)
        for (int i = 0; i < length; i++) {
            /* Complex index calculation to prevent optimization */
            int idx = low + ((i * 17) % length);
            section_z[i] = section_x[idx] * local_scale + 
                          section_y[(i + 32) % length] / local_scale;
        }
    }
}

/* Variant 3: Combined constructs with pointer arithmetic */
void combined_constructs(double *restrict d1, double *restrict d2,
                        double *restrict d3, int offset, 
                        volatile int iter_count) {
    const double pi = 3.141592653589793;
    double *ptr1 = d1 + offset;
    double *ptr2 = d2 + offset;
    double *ptr3 = d3 + offset;
    int count = iter_count;
    
    #pragma omp target data map(to: ptr1[0:N/2], ptr2[0:N/2]) \
                            map(from: ptr3[0:N/2])
    {
        #pragma omp target teams distribute parallel for simd \
            collapse(2) schedule(static, 16) \
            firstprivate(pi, count, offset)
        for (int block = 0; block < count; block++) {
            for (int elem = 0; elem < 32; elem++) {
                int idx = offset + block * 32 + elem;
                if (idx < N) {
                    /* Complex computation with branches */
                    double val = ptr1[idx];
                    if (val > 0.5) {
                        ptr3[idx] = val * pi + ptr2[idx];
                    } else {
                        ptr3[idx] = val / pi - ptr2[idx];
                    }
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    int local_factor = factor;
    #pragma omp parallel for simd schedule(dynamic, 8)
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * local_factor) % 997;
    }
}

/* Function to select between target and host execution */
void conditional_execution(int *a, int *b, int *c, int size,
                          int use_target, volatile int seed) {
    if (use_target) {
        simd_target_loop(a, b, c, 0, size, 1, size);
    } else {
        host_only_parallel(a, size, seed);
        #pragma omp simd
        for (int i = 0; i < size; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Declare arrays with different storage durations */
    static int static_array[N];
    int auto_array[N];
    float float_array[M];
    double double_array[N];
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 1000;
        auto_array[i] = rand() % 1000;
        double_array[i] = (double)rand() / RAND_MAX;
    }
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)rand() / RAND_MAX;
    }
    
    /* Additional arrays for results */
    int result1[N], result2[N];
    float result_float[M];
    double result_double[N];
    
    /* Volatile variables to prevent constant folding */
    volatile int v_size = N;
    volatile float v_scale = 2.5f + (rand() % 100) * 0.01f;
    volatile int v_iter = 3 + (rand() % 3);
    
    printf("Starting OpenMP SIMT transformation test (seed: %d)\n", seed);
    
    /* Main test loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("\nIteration %d:\n", iter);
        
        /* Vary parameters each iteration */
        int use_target = (iter % 2 == 0) ? 1 : 0;
        int offset = (iter * 128) % N;
        int stride = 1 + (iter % 3);
        
        /* Test Variant 1 with conditional execution */
        conditional_execution(static_array, auto_array, result1, 
                            v_size, use_target, seed + iter);
        
        /* Calculate checksum to prevent dead code elimination */
        long long checksum1 = 0;
        #pragma omp parallel for reduction(+:checksum1)
        for (int i = 0; i < v_size; i++) {
            checksum1 += result1[i];
        }
        printf("  Checksum1: %lld\n", checksum1);
        
        /* Test Variant 2 with array sections */
        parallel_target_loop(float_array, float_array + M/2, 
                           result_float, 0, M/2, v_scale);
        
        /* Test Variant 3 with complex data mapping */
        combined_constructs(double_array, double_array + N/4,
                          result_double, offset, v_iter);
        
        /* Verify results with simple computation */
        double checksum3 = 0.0;
        #pragma omp simd reduction(+:checksum3)
        for (int i = 0; i < N/2; i++) {
            checksum3 += result_double[i];
        }
        printf("  Checksum3: %.6f\n", checksum3);
        
        /* Modify volatile variables for next iteration */
        v_size = (v_size + 64) % N;
        v_scale = v_scale * 0.9f;
    }
    
    /* Final verification with all arrays */
    printf("\nFinal verification:\n");
    
    #pragma omp target teams distribute parallel for simd \
        map(to: static_array[0:N], auto_array[0:N]) \
        map(from: result2[0:N])
    for (int i = 0; i < N; i++) {
        /* Complex index calculation */
        int idx = (i * 7 + 13) % N;
        result2[i] = static_array[idx] * 3 - auto_array[(i + 5) % N];
    }
    
    long long final_checksum = 0;
    #pragma omp parallel for simd reduction(+:final_checksum)
    for (int i = 0; i < N; i++) {
        final_checksum += result2[i];
    }
    printf("Final checksum: %lld\n", final_checksum);
    
    return 0;
}
