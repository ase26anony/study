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
                      volatile int n, volatile int m) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end-start], b[start:end-start]) \
        map(from: c[start:end-start]) \
        collapse(2) if(end-start > 100)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx >= start && idx < end) {
                c[idx] = a[idx] * step + b[idx] / (step + 1);
            }
        }
    }
}

// Variant 2: Parallel target loop without SIMD
void parallel_target_loop(float *x, float *y, float *z, int low, int high, 
                          int stride, volatile int limit) {
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high-low:stride], y[low:high-low:stride]) \
        map(from: z[low:high-low:stride]) \
        private(low, high) firstprivate(stride) shared(limit)
    for (int i = low; i < high; i += stride) {
        if (i < limit) {
            z[i] = x[i] * 2.5f - y[i] / 1.5f;
        }
    }
}

// Variant 3: Combined constructs with data region
void combined_constructs(double *p, double *q, double *r, int size, 
                         volatile int offset, volatile int block) {
    static double local_buffer[256];
    const double scale = 3.14159;
    
    #pragma omp target data map(to: p[0:size], q[0:size]) map(from: r[0:size])
    {
        #pragma omp target teams distribute parallel for simd \
            if(size > 200) collapse(2) \
            num_teams(block) thread_limit(128)
        for (int i = 0; i < size/block; i++) {
            for (int j = 0; j < block; j++) {
                int idx = i * block + j;
                if (idx < size) {
                    double temp = p[idx] * scale + q[idx] / scale;
                    local_buffer[j % 256] = temp;  // Use static array
                    r[idx] = temp + offset + local_buffer[j % 256];
                }
            }
        }
    }
}

// Host-only parallel region for conditional execution
void host_only_parallel(int *arr, int n, int factor) {
    #pragma omp parallel for simd if(n > 50)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * factor + i;
    }
}

// Function with mixed storage durations
void complex_data_env(volatile int seed, int iter) {
    static int static_arr[N];
    const int const_size = M;
    int auto_arr[N];
    int *dyn_arr = (int*)malloc(N * sizeof(int));
    
    // Initialize arrays with pattern
    for (int i = 0; i < N; i++) {
        static_arr[i] = (i + seed) % 100;
        auto_arr[i] = (i * iter) % 100;
        dyn_arr[i] = (i + seed * iter) % 100;
    }
    
    int result[N];
    
    // Use command-line dependent condition
    if (seed % 3 == 0) {
        // Call target region
        #pragma omp target teams distribute parallel for simd \
            map(to: static_arr[0:N], auto_arr[0:N], dyn_arr[0:N]) \
            map(from: result[0:N]) \
            collapse(2) if(N > 500)
        for (int i = 0; i < const_size; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < N) {
                    result[idx] = static_arr[idx] + auto_arr[idx] * dyn_arr[idx];
                }
            }
        }
    } else {
        // Call host-only parallel
        host_only_parallel(result, N, seed);
    }
    
    // Verify with checksum
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += result[i];
    }
    printf("Checksum for iter %d: %d\n", iter, checksum);
    
    free(dyn_arr);
}

int main(int argc, char *argv[]) {
    // Use command-line argument for variability
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different types
    int arr1[N], arr2[N], arr3[N];
    float farr1[M], farr2[M], farr3[M];
    double darr1[N], darr2[N], darr3[N];
    
    // Initialize with random data
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = 0;
        darr1[i] = (double)(rand() % 100) / 10.0;
        darr2[i] = (double)(rand() % 100) / 10.0;
        darr3[i] = 0.0;
    }
    
    for (int i = 0; i < M; i++) {
        farr1[i] = (float)(rand() % 100) / 5.0f;
        farr2[i] = (float)(rand() % 100) / 5.0f;
        farr3[i] = 0.0f;
    }
    
    // Main loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int bound1 = 100 + rand() % 900;
        volatile int bound2 = 50 + rand() % 450;
        volatile int step = 1 + rand() % 4;
        volatile int offset = rand() % 100;
        volatile int block = 16 + rand() % 48;
        
        printf("\n=== Iteration %d ===\n", iter);
        
        // Call variant functions with different parameters
        if (iter % 3 == 0) {
            simd_target_loop(arr1, arr2, arr3, 0, bound1, step, N, M);
            
            // Verify results
            int sum = 0;
            for (int i = 0; i < bound1; i++) {
                sum += arr3[i];
            }
            printf("SIMD target checksum: %d\n", sum);
        }
        
        if (iter % 3 == 1) {
            int low = rand() % 100;
            int high = low + bound2;
            parallel_target_loop(farr1, farr2, farr3, low, high, step, M);
            
            // Verify results
            float fsum = 0.0f;
            for (int i = low; i < high; i += step) {
                fsum += farr3[i];
            }
            printf("Parallel target checksum: %.2f\n", fsum);
        }
        
        if (iter % 3 == 2) {
            int size = 200 + rand() % 800;
            combined_constructs(darr1, darr2, darr3, size, offset, block);
            
            // Verify results
            double dsum = 0.0;
            for (int i = 0; i < size; i++) {
                dsum += darr3[i];
            }
            printf("Combined constructs checksum: %.2f\n", dsum);
        }
        
        // Call function with complex data environment
        complex_data_env(seed + iter, iter);
    }
    
    // Additional test with pointer arithmetic
    int *ptr1 = arr1 + 100;
    int *ptr2 = arr2 + 100;
    int *ptr3 = arr3 + 100;
    int section_size = 200;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: ptr1[0:section_size], ptr2[0:section_size]) \
        map(from: ptr3[0:section_size]) \
        if(section_size > 100)
    for (int i = 0; i < section_size; i++) {
        ptr3[i] = ptr1[i] * ptr2[i] - i;
    }
    
    // Final verification
    int final_sum = 0;
    for (int i = 100; i < 100 + section_size; i++) {
        final_sum += arr3[i];
    }
    printf("\nFinal pointer arithmetic checksum: %d\n", final_sum);
    
    return 0;
}
