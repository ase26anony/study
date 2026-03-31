#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

// Variant 1: SIMD target loop
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int bound1, volatile int bound2) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end], b[start:end]) map(from: c[start:end]) \
        firstprivate(start, end, step) collapse(2) if(target: bound1 > 100)
    for (int i = start; i < end; i += step) {
        for (int j = 0; j < bound2 % 16; j++) {
            int idx = i + j;
            if (idx < end) {
                c[idx] = a[idx] + b[idx] * (j + 1);
            }
        }
    }
}

// Variant 2: Parallel target loop without SIMD
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                         int stride, volatile int cond) {
    static const float scale = 2.5f;
    float local_scale = (float)(cond % 10) / 10.0f + 1.0f;
    
    #pragma omp target data map(to: x[low:high:stride], y[low:high:stride]) \
                            map(from: z[low:high:stride]) if(target: cond > 50)
    {
        #pragma omp target teams distribute parallel for \
            firstprivate(local_scale) private(scale) shared(x, y, z) \
            num_teams(cond % 8 + 1) thread_limit(64)
        for (int i = low; i < high; i += stride) {
            z[i] = x[i] * scale + y[i] * local_scale;
        }
    }
}

// Variant 3: Combined constructs with complex data environment
void combined_constructs(double *p, double *q, double *r, int dim1, int dim2,
                        volatile int flag, volatile int *dynamic_bound) {
    const int chunk = 32;
    int *private_arr = (int*)malloc(chunk * sizeof(int));
    
    // Initialize private array with pattern
    for (int k = 0; k < chunk; k++) {
        private_arr[k] = k * (flag % 7 + 1);
    }
    
    #pragma omp target data map(to: p[0:dim1*dim2], q[0:dim1*dim2]) \
                            map(from: r[0:dim1*dim2]) \
                            map(tofrom: dynamic_bound[0:1])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(private_arr[0:chunk]) collapse(2) \
            if(target: *dynamic_bound > 0)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                int mod = private_arr[j % chunk];
                r[idx] = p[idx] * (mod + 1) - q[idx] / (mod + 1);
            }
        }
    }
    
    free(private_arr);
}

// Variant 4: Nested function with pointer arithmetic
void nested_pointer_ops(short *src, short *dst, int n, volatile int offset) {
    short *local_src = src + offset;
    short *local_dst = dst + offset;
    int limit = n - offset;
    
    if (limit > 0) {
        #pragma omp target teams distribute parallel for simd \
            map(to: local_src[0:limit]) map(from: local_dst[0:limit]) \
            firstprivate(offset) if(target: offset % 3 == 0)
        for (int i = 0; i < limit; i++) {
            local_dst[i] = local_src[i] * (i % 16 + 1) - offset;
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int size, volatile int seed) {
    #pragma omp parallel for simd if(seed > 1000)
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * (seed % 5 + 1) + i;
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for random seed
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N];
    int auto_array[N];
    const int const_size = M;
    float float_array[M];
    double double_array[N * M / 4];
    short short_array[N * 2];
    
    // Initialize arrays with random data
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        if (i < M) {
            float_array[i] = (float)(rand() % 1000) / 10.0f;
        }
        if (i < N * 2) {
            short_array[i] = (short)(rand() % 256);
        }
    }
    
    for (int i = 0; i < N * M / 4; i++) {
        double_array[i] = (double)(rand() % 10000) / 100.0;
    }
    
    // Result arrays
    int result_int[N];
    float result_float[M];
    double result_double[N * M / 4];
    short result_short[N * 2];
    memset(result_int, 0, sizeof(result_int));
    memset(result_float, 0, sizeof(result_float));
    memset(result_double, 0, sizeof(result_double));
    memset(result_short, 0, sizeof(result_short));
    
    // Volatile variables to prevent constant folding
    volatile int v_bound1 = rand() % 200;
    volatile int v_bound2 = rand() % 100;
    volatile int v_cond = rand() % 100;
    volatile int v_flag = rand() % 20;
    volatile int v_offset = rand() % 50;
    volatile int *dynamic_bound = (volatile int*)malloc(sizeof(int));
    *dynamic_bound = rand() % 100;
    
    // Main execution loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("Iteration %d:\n", iter);
        
        // Randomly choose between target and host execution
        int choice = rand() % 3;
        
        if (choice == 0) {
            // Call target variant functions
            int start = rand() % (N / 2);
            int end = start + (rand() % (N / 4)) + 100;
            int step = (rand() % 3) + 1;
            
            simd_target_loop(static_array, auto_array, result_int, 
                           start, end, step, v_bound1, v_bound2);
            
            // Verify results
            long long checksum = 0;
            for (int i = start; i < end && i < N; i += step) {
                checksum += result_int[i];
            }
            printf("  SIMD target checksum: %lld\n", checksum);
            
        } else if (choice == 1) {
            // Mixed execution path
            int low = rand() % (M / 2);
            int high = low + (rand() % (M / 4)) + 50;
            int stride = (rand() % 2) + 1;
            
            parallel_target_loop(float_array, float_array, result_float,
                               low, high, stride, v_cond);
            
            // Also call host-only version
            host_only_parallel(result_int, N / 2, rand());
            
            // Verify results
            float fchecksum = 0.0f;
            for (int i = low; i < high && i < M; i += stride) {
                fchecksum += result_float[i];
            }
            printf("  Parallel target checksum: %.2f\n", fchecksum);
            
        } else {
            // Complex combined constructs
            int dim1 = (rand() % 8) + 4;
            int dim2 = (rand() % 16) + 8;
            int total = dim1 * dim2;
            
            if (total <= N * M / 4) {
                combined_constructs(double_array, double_array, result_double,
                                  dim1, dim2, v_flag, (int*)dynamic_bound);
                
                // Verify results
                double dchecksum = 0.0;
                for (int i = 0; i < total && i < N * M / 4; i++) {
                    dchecksum += result_double[i];
                }
                printf("  Combined constructs checksum: %.2f\n", dchecksum);
            }
            
            // Nested pointer operations
            nested_pointer_ops(short_array, result_short, N * 2, v_offset);
            
            // Verify results
            long schecksum = 0;
            int limit = (N * 2) - v_offset;
            if (limit > 0) {
                for (int i = 0; i < limit && i < N * 2; i++) {
                    schecksum += result_short[i];
                }
            }
            printf("  Pointer ops checksum: %ld\n", schecksum);
        }
        
        // Update volatile variables for next iteration
        v_bound1 = (v_bound1 * 13 + 17) % 200;
        v_bound2 = (v_bound2 * 7 + 23) % 100;
        v_cond = (v_cond * 11 + 29) % 100;
        *dynamic_bound = (*dynamic_bound * 5 + 7) % 100;
    }
    
    free((void*)dynamic_bound);
    
    // Final verification
    printf("\nFinal verification:\n");
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += result_int[i] % 256;
    }
    printf("Final integer result sum: %d\n", final_sum);
    
    return 0;
}
