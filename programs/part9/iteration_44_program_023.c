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
    int private_var = start;
    
    #pragma omp target teams distribute parallel for simd \
            collapse(2) private(private_var) firstprivate(chunk_size) \
            map(to: a[start:end:stride], b[start:end:stride]) \
            map(tofrom: c[start:end:stride]) \
            num_teams(m/64) thread_limit(128)
    for (int i = 0; i < n; i += chunk_size) {
        for (int j = 0; j < chunk_size; ++j) {
            int idx = i + j;
            if (idx < n) {
                private_var = idx % 16;
                c[idx] = a[idx] * private_var + b[idx] / (private_var + 1);
            }
        }
    }
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, int low, int high,
                          volatile int limit, int offset) {
    float local_accum = 0.0f;
    const float scale = 2.5f;
    
    #pragma omp target teams distribute parallel for \
            map(to: x[low:high], y[low:high]) \
            map(tofrom: z[low:high]) reduction(+:local_accum) \
            num_teams(limit/32) thread_limit(64)
    for (int i = low; i < high; i++) {
        float temp = x[i] * scale + y[i];
        z[i] = temp * (i % 8 + 1);
        local_accum += z[i];
    }
    
    /* Use result to prevent elimination */
    z[low] += local_accum;
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *p, double *q, double *r, int size,
                         volatile int seed, int mode) {
    double *tmp = (double*)malloc(size * sizeof(double));
    const double pi = 3.1415926535;
    
    #pragma omp target data map(to: p[0:size], q[0:size]) \
                            map(from: r[0:size], tmp[0:size])
    {
        int block = size / 4;
        
        #pragma omp target teams distribute parallel for simd \
                map(alloc: tmp[0:size]) \
                num_teams(block) thread_limit(256)
        for (int i = 0; i < size; i++) {
            double angle = (i + seed) * pi / size;
            tmp[i] = p[i] * cos(angle) + q[i] * sin(angle);
            
            /* Complex indexing with pointer arithmetic */
            double *ptr = r + (i % block);
            *ptr = tmp[i] * (mode + 1);
            
            if (i % 3 == 0) {
                r[i] = tmp[i] * 0.5;
            } else if (i % 3 == 1) {
                r[i] = tmp[i] * 1.5;
            } else {
                r[i] = tmp[i] * 2.5;
            }
        }
    }
    
    free(tmp);
}

/* Variant 4: Nested function with conditional target execution */
void nested_target_region(int *arr1, int *arr2, int dim1, int dim2,
                          volatile int threshold) {
    int local_arr[16];
    for (int k = 0; k < 16; k++) local_arr[k] = k;
    
    /* Runtime condition that may affect SIMT transformation */
    if (threshold > 100) {
        #pragma omp target teams distribute parallel for simd \
                collapse(2) map(to: arr1[0:dim1*dim2]) \
                map(tofrom: arr2[0:dim1*dim2]) \
                firstprivate(local_arr)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                int mod = local_arr[idx % 16];
                arr2[idx] = arr1[idx] * mod + (i * j) % 7;
            }
        }
    } else {
        /* Host-only parallel region for comparison */
        #pragma omp parallel for simd
        for (int i = 0; i < dim1 * dim2; i++) {
            arr2[i] = arr1[i] * 2;
        }
    }
}

/* Host-only function to create variability */
void host_only_computation(int *data, int n, volatile int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * factor + i;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Volatile variables to prevent constant folding */
    volatile int v_size = N + (rand() % 100);
    volatile int v_limit = M + (rand() % 50);
    volatile int v_threshold = rand() % 200;
    
    /* Arrays with different storage durations */
    static int static_array[N];
    int auto_array[N];
    const int const_size = N;
    
    float float_array[M];
    double double_array[N];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < N; i++) {
        static_array[i] = i * 2;
        auto_array[i] = i * 3;
        double_array[i] = i * 1.5;
    }
    
    for (int i = 0; i < M; i++) {
        float_array[i] = i * 0.7f;
    }
    
    int checksum = 0;
    
    /* Loop with varying parameters to expose different contexts */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        int mode = iter % 3;
        int offset = (rand() % 100);
        int stride = 1 + (iter % 4);
        
        /* Call different variants based on runtime conditions */
        if (iter % 2 == 0) {
            simd_target_loop(static_array, auto_array, auto_array,
                            offset, v_size, stride, v_size, v_limit);
        } else {
            parallel_target_loop(float_array, float_array, float_array,
                               0, M, v_limit, offset);
        }
        
        if (v_threshold > 150 || iter == 2) {
            combined_constructs(double_array, double_array, double_array,
                               N, seed + iter, mode);
        }
        
        /* Mix with host-only computations */
        if (rand() % 3 == 0) {
            host_only_computation(auto_array, N, v_threshold);
        }
        
        /* Nested region with conditional execution */
        nested_target_region(static_array, auto_array, 32, 32, v_threshold);
        
        /* Update checksum to prevent elimination */
        for (int i = 0; i < 10; i++) {
            checksum += static_array[i] + auto_array[i];
            checksum += (int)float_array[i % M];
            checksum += (int)double_array[i];
        }
        
        /* Modify volatile variables for next iteration */
        v_threshold += rand() % 20;
        v_limit -= rand() % 10;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Seed used: %d\n", seed);
    
    /* Verify results with simple test */
    int verify_sum = 0;
    #pragma omp parallel for reduction(+:verify_sum)
    for (int i = 0; i < 100; i++) {
        verify_sum += static_array[i] + auto_array[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    return 0;
}
