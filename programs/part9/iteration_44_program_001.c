#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, 
                      int stride, volatile int n_iter) {
    static int static_counter = 0;
    const int const_offset = 10;
    int private_var = start;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end-start:stride], b[start:end-start:stride]) \
        map(from: c[start:end-start:stride]) \
        firstprivate(private_var, const_offset) \
        shared(static_counter) \
        collapse(2) \
        num_teams(end-start) \
        thread_limit(256)
    for (int i = start; i < end; i += stride) {
        for (int j = 0; j < n_iter; j++) {
            int idx = i * M + j;
            if (idx < N * M) {
                c[idx] = a[idx] + b[idx] * (private_var + const_offset + j);
                #pragma omp atomic
                static_counter++;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, volatile float scale) {
    float local_scale = scale;
    float *ptr = &local_scale;
    
    #pragma omp target data map(to: x[low:high-low], y[low:high-low]) \
                            map(from: z[low:high-low])
    {
        #pragma omp target teams distribute parallel for \
            firstprivate(local_scale) \
            private(ptr) \
            num_teams(high-low) \
            thread_limit(128)
        for (int i = low; i < high; i++) {
            ptr = &local_scale;
            z[i] = x[i] * (*ptr) + y[i];
            
            // Complex indexing to prevent optimization
            if (i % 3 == 0) {
                z[i] += x[(i + 1) % high] * 0.5f;
            } else if (i % 7 == 0) {
                z[i] -= y[(i * 2) % high] * 0.25f;
            }
        }
    }
}

/* Variant 3: Combined constructs with pointer arithmetic */
void combined_constructs(double *d1, double *d2, double *result,
                        int base, int length, volatile int mode) {
    const int chunk = 64;
    double *slice1 = d1 + base;
    double *slice2 = d2 + base;
    double *res_slice = result + base;
    
    #pragma omp target data map(to: slice1[0:length], slice2[0:length]) \
                            map(from: res_slice[0:length])
    {
        #pragma omp target teams distribute parallel for simd \
            firstprivate(mode, chunk) \
            collapse(2) \
            num_teams((length + chunk - 1) / chunk)
        for (int i = 0; i < length; i += chunk) {
            for (int j = 0; j < chunk && (i + j) < length; j++) {
                int idx = i + j;
                switch (mode % 3) {
                    case 0:
                        res_slice[idx] = slice1[idx] * slice2[idx];
                        break;
                    case 1:
                        res_slice[idx] = slice1[idx] + slice2[idx];
                        break;
                    case 2:
                        res_slice[idx] = slice1[idx] - slice2[idx];
                        break;
                }
                
                // Additional computation to create data dependencies
                if (idx > 0) {
                    res_slice[idx] += res_slice[idx - 1] * 0.1;
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    int local_factor = factor;
    
    #pragma omp parallel for simd firstprivate(local_factor)
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * local_factor + i;
    }
}

/* Function that conditionally calls target or host version */
void conditional_execution(int *a, int *b, int *c, int size, 
                          volatile int use_target, volatile int seed) {
    srand(seed);
    
    if (use_target && (rand() % 100) > 30) {
        // Call target version with varying parameters
        int start = rand() % (size / 2);
        int end = start + (rand() % (size / 2)) + 1;
        int stride = (rand() % 3) + 1;
        
        simd_target_loop(a, b, c, start, end, stride, rand() % 10 + 1);
    } else {
        // Call host version
        host_only_parallel(a, size, rand() % 10 + 1);
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    // Declare arrays with different storage durations
    static int static_array[N * M];
    int auto_array[N * M];
    float float_array[N];
    double double_array[N];
    volatile int vol_bound = N;
    
    // Initialize arrays with random data
    for (int i = 0; i < N * M; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
    }
    
    for (int i = 0; i < N; i++) {
        float_array[i] = (float)rand() / RAND_MAX;
        double_array[i] = (double)rand() / RAND_MAX;
    }
    
    int result_array[N * M] = {0};
    float result_float[N] = {0.0f};
    double result_double[N] = {0.0};
    
    // Main execution loop with varying parameters
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int mode = rand() % 3;
        volatile int use_simd = (iter % 2 == 0);
        volatile int bound = (rand() % (N/2)) + (N/4);
        
        printf("Iteration %d: mode=%d, use_simd=%d, bound=%d\n", 
               iter, mode, use_simd, bound);
        
        // Call variant 1 with different slices
        int start = rand() % (N/2);
        int end = start + bound;
        int stride = (rand() % 4) + 1;
        
        simd_target_loop(static_array, auto_array, result_array, 
                        start, end, stride, rand() % 5 + 1);
        
        // Calculate checksum for verification
        long long checksum1 = 0;
        for (int i = start; i < end && i < N * M; i += stride) {
            checksum1 += result_array[i];
        }
        printf("  Checksum1: %lld\n", checksum1);
        
        // Call variant 2 with random bounds
        int low = rand() % (N/2);
        int high = low + (rand() % (N/2)) + 1;
        volatile float scale = (float)(rand() % 100) / 10.0f;
        
        parallel_target_loop(float_array, float_array, result_float,
                           low, high, scale);
        
        // Calculate checksum for float results
        float checksum2 = 0.0f;
        for (int i = low; i < high && i < N; i++) {
            checksum2 += result_float[i];
        }
        printf("  Checksum2: %f\n", checksum2);
        
        // Call variant 3 with combined constructs
        int base = rand() % (N/2);
        int length = (rand() % (N/2)) + 1;
        volatile int comp_mode = rand() % 3;
        
        combined_constructs(double_array, double_array, result_double,
                          base, length, comp_mode);
        
        // Calculate checksum for double results
        double checksum3 = 0.0;
        for (int i = base; i < base + length && i < N; i++) {
            checksum3 += result_double[i];
        }
        printf("  Checksum3: %f\n", checksum3);
        
        // Conditional execution based on random factors
        conditional_execution(static_array, auto_array, result_array,
                            N * M, use_simd, seed + iter);
        
        printf("\n");
    }
    
    return 0;
}
