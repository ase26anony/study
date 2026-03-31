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
        collapse(2) num_teams(bound1) thread_limit(bound2)
    for (int i = start; i < end; i += step) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            if (idx < end * M) {
                c[idx] = a[idx] + b[idx] * (i % 8 + j % 4);
            }
        }
    }
}

// Variant 2: Parallel target loop without SIMD
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                         int stride, volatile int teams, volatile int threads) {
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:stride], y[low:high:stride]) \
        map(from: z[low:high:stride]) \
        num_teams(teams) thread_limit(threads)
    for (int i = low; i < high; i += stride) {
        float sum = 0.0f;
        for (int k = 0; k < 16; k++) {
            sum += x[i + k] * y[i - k + 15];
        }
        z[i] = sum * (i % 3 + 1);
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *p, double *q, double *r, int size, 
                        volatile int seed, volatile int mode) {
    static double local_buffer[256];
    const double scale = 2.5;
    
    #pragma omp target data map(to: p[0:size], q[0:size]) map(from: r[0:size])
    {
        // Initialize local buffer with varying pattern
        #pragma omp parallel for simd
        for (int i = 0; i < 256; i++) {
            local_buffer[i] = (i * seed) % 100;
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: local_buffer[0:256]) \
            private(scale) firstprivate(seed) shared(p, q, r)
        for (int i = 0; i < size; i++) {
            double temp = p[i] * q[i];
            if (mode == 0) {
                r[i] = temp + local_buffer[i % 256] * scale;
            } else {
                r[i] = temp - local_buffer[i % 256] / scale;
            }
        }
    }
}

// Variant 4: Host-only parallel region (for conditional execution)
void host_only_parallel(int *arr1, int *arr2, int n, volatile int flag) {
    #pragma omp parallel for simd if(flag > 0)
    for (int i = 0; i < n; i++) {
        arr1[i] = arr2[i] * (i % 7 + 1);
    }
}

// Helper function with pointer arithmetic
void process_section(int *base, int offset, int length, int stride, 
                    volatile int modifier) {
    int *section = base + offset;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: section[0:length:stride]) \
        collapse(2)
    for (int i = 0; i < length; i += stride) {
        for (int j = 0; j < 4; j++) {
            int idx = i + j;
            if (idx < length) {
                section[idx] = (section[idx] + modifier) * (i % 5 + j % 3);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Use command-line argument for seed
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N * M];
    int auto_array[N * M];
    const int const_size = N * M;
    volatile int vol_bound = rand() % 100 + 50;
    
    // Initialize arrays
    for (int i = 0; i < N * M; i++) {
        static_array[i] = (i * 3) % 100;
        auto_array[i] = (i * 7) % 100;
    }
    
    float float_array[N];
    double double_array[N];
    for (int i = 0; i < N; i++) {
        float_array[i] = (float)(i * 11) / 100.0f;
        double_array[i] = (double)(i * 13) / 100.0;
    }
    
    int result_array[N * M];
    float result_float[N];
    double result_double[N];
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int mode = rand() % 3;
        volatile int use_target = (rand() % 2) && (argc > 1);
        volatile int bound1 = rand() % 8 + 2;
        volatile int bound2 = rand() % 64 + 32;
        
        printf("Iteration %d, mode=%d, use_target=%d\n", 
               iter, mode, use_target);
        
        if (use_target) {
            // Call target region functions
            int start = iter * 100;
            int end = start + 200;
            int step = (iter % 2) + 1;
            
            simd_target_loop(static_array, auto_array, result_array,
                           start, end, step, bound1, bound2);
            
            // Calculate checksum
            long long checksum = 0;
            for (int i = start; i < end && i < N * M; i++) {
                checksum += result_array[i];
            }
            printf("  SIMD target checksum: %lld\n", checksum);
            
            // Process with pointer arithmetic
            process_section(result_array, iter * 50, 150, 2, iter * 10);
            
            // Combined constructs
            int size = N - iter * 50;
            if (size > 0) {
                combined_constructs(double_array, double_array + iter * 10,
                                  result_double, size, seed + iter, mode);
                
                double dsum = 0.0;
                for (int i = 0; i < size; i++) {
                    dsum += result_double[i];
                }
                printf("  Combined constructs sum: %.2f\n", dsum);
            }
        } else {
            // Host-only path
            host_only_parallel(static_array, auto_array, 
                             (iter + 1) * 100, mode);
            
            // Still call some target functions to expose compiler to both paths
            if (iter % 2 == 0) {
                parallel_target_loop(float_array, float_array + N/2,
                                   result_float, 0, N/2, 2, bound1, bound2);
                
                float fsum = 0.0f;
                for (int i = 0; i < N/2; i++) {
                    fsum += result_float[i];
                }
                printf("  Parallel target sum: %.2f\n", fsum);
            }
        }
        
        // Mix array sections with different strides
        int section_start = iter * 64;
        int section_len = 128;
        int section_stride = (iter % 3) + 1;
        
        #pragma omp target teams distribute parallel for simd \
            map(to: static_array[section_start:section_len:section_stride]) \
            map(from: auto_array[section_start:section_len:section_stride])
        for (int i = 0; i < section_len; i += section_stride) {
            int idx = section_start + i;
            if (idx < N * M) {
                auto_array[idx] = static_array[idx] * (i % 9 + 2);
            }
        }
    }
    
    // Final verification
    long long final_sum = 0;
    #pragma omp parallel for simd reduction(+:final_sum)
    for (int i = 0; i < N * M; i++) {
        final_sum += static_array[i] + auto_array[i];
    }
    printf("Final array sum: %lld\n", final_sum);
    
    return 0;
}
