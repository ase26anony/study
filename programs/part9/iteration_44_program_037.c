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
    const int chunk_size = 16;
    int local_n = n;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) num_teams(8) thread_limit(64) \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        private(local_n) firstprivate(chunk_size) \
        shared(static_counter)
    for (int i = 0; i < local_n; i += chunk_size) {
        for (int j = 0; j < chunk_size && (i + j) < local_n; j++) {
            int idx = start + (i + j) * stride;
            c[idx] = a[idx] * 2 + b[idx] / 3;
            static_counter++;
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *restrict x, float *restrict y, 
                          float *restrict z, int low, int high,
                          volatile float scale) {
    float local_scale = scale;
    float *ptr_x = x + low;
    float *ptr_y = y + low;
    float *ptr_z = z + low;
    int length = high - low;
    
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(128) \
        map(to: ptr_x[0:length], ptr_y[0:length]) \
        map(from: ptr_z[0:length]) \
        firstprivate(local_scale, length)
    for (int i = 0; i < length; i++) {
        ptr_z[i] = ptr_x[i] * local_scale + ptr_y[i] / local_scale;
        /* Complex indexing to prevent optimization */
        if (i % 32 == 0) {
            ptr_z[i] += (float)(i & 0xFF);
        }
    }
}

/* Variant 3: Combined constructs with data region */
void combined_constructs(double *restrict d1, double *restrict d2,
                         double *restrict d3, int dim1, int dim2,
                         volatile int mode) {
    int local_mode = mode;
    const double pi = 3.141592653589793;
    double *temp = (double*)malloc(dim1 * dim2 * sizeof(double));
    
    #pragma omp target data map(to: d1[0:dim1*dim2], d2[0:dim1*dim2]) \
                            map(from: d3[0:dim1*dim2]) \
                            map(alloc: temp[0:dim1*dim2])
    {
        /* Intermediate computation */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) num_teams(16) thread_limit(32)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                int idx = i * dim2 + j;
                temp[idx] = d1[idx] * d2[idx] + pi * (i + j);
            }
        }
        
        /* Conditional second computation */
        if (local_mode > 0) {
            #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(64)
            for (int i = 0; i < dim1 * dim2; i++) {
                d3[i] = temp[i] * 0.5 + d1[i] * 0.3 - d2[i] * 0.2;
            }
        } else {
            #pragma omp target teams distribute parallel for simd \
                num_teams(4) thread_limit(256)
            for (int i = 0; i < dim1 * dim2; i++) {
                d3[i] = temp[i] * 2.0 - d1[i] * 1.5 + d2[i] * 0.8;
            }
        }
    }
    
    free(temp);
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, volatile int factor) {
    int local_factor = factor;
    #pragma omp parallel for simd schedule(dynamic, 8)
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * local_factor) % 997;
        /* Prevent vectorization elimination */
        if (i % 17 == 0) {
            arr[i] ^= 0x5A5A5A5A;
        }
    }
}

/* Function that selects between target and host execution */
void conditional_execution(int *data, int len, int use_target, 
                           volatile int seed) {
    if (use_target) {
        int *copy1 = (int*)malloc(len * sizeof(int));
        int *copy2 = (int*)malloc(len * sizeof(int));
        memcpy(copy1, data, len * sizeof(int));
        memcpy(copy2, data, len * sizeof(int));
        
        #pragma omp target teams distribute parallel for simd \
            map(to: copy1[0:len]) map(from: copy2[0:len]) \
            firstprivate(seed)
        for (int i = 0; i < len; i++) {
            copy2[i] = copy1[i] * (seed % 7 + 1) + (i & 0xF);
        }
        
        memcpy(data, copy2, len * sizeof(int));
        free(copy1);
        free(copy2);
    } else {
        host_only_parallel(data, len, seed);
    }
}

int main(int argc, char *argv[]) {
    /* Use volatile for critical values to prevent constant folding */
    volatile int seed = 42;
    volatile int use_simt_variant = 1;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
        use_simt_variant = (seed % 3) != 0;
    }
    
    srand(seed);
    
    /* Mixed storage duration arrays */
    static int static_array[N];
    int auto_array[N];
    float float_array[M];
    double double_array[N * M / 4];
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 1000;
        auto_array[i] = rand() % 1000;
    }
    
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < N * M / 4; i++) {
        double_array[i] = (double)(rand() % 1000) / 3.0;
    }
    
    int checksum = 0;
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int current_seed = seed + iter * 17;
        volatile int loop_bound = N - iter * 50;
        volatile float scale_factor = 1.0f + iter * 0.3f;
        
        /* Call variant functions with different parameters */
        if (use_simt_variant || (iter % 2 == 0)) {
            int start = iter * 32;
            int end = loop_bound;
            int stride = 1 + (iter % 4);
            
            int *temp_c = (int*)malloc(N * sizeof(int));
            simd_target_loop(static_array, auto_array, temp_c, 
                           start, end, stride, loop_bound);
            
            /* Compute checksum to prevent dead code elimination */
            for (int i = start; i < end; i += stride) {
                checksum += temp_c[i];
            }
            free(temp_c);
        }
        
        /* Alternate between different OpenMP constructs */
        if (iter % 3 != 0) {
            float *temp_z = (float*)malloc(M * sizeof(float));
            int low = iter * 16;
            int high = M - iter * 8;
            
            parallel_target_loop(float_array, float_array, temp_z,
                               low, high, scale_factor);
            
            for (int i = low; i < high; i++) {
                checksum += (int)temp_z[i];
            }
            free(temp_z);
        }
        
        /* Combined constructs with varying dimensions */
        if (iter < 3) {
            int dim1 = 64 + iter * 32;
            int dim2 = 32 + iter * 16;
            double *temp_d3 = (double*)malloc(dim1 * dim2 * sizeof(double));
            
            combined_constructs(double_array, double_array + dim1 * dim2,
                              temp_d3, dim1, dim2, iter);
            
            for (int i = 0; i < dim1 * dim2; i += 8) {
                checksum += (int)(temp_d3[i] * 100);
            }
            free(temp_d3);
        }
        
        /* Conditional execution based on runtime value */
        int use_target = (current_seed % 5) > 2;
        int *work_data = (int*)malloc(N * sizeof(int));
        memcpy(work_data, static_array, N * sizeof(int));
        
        conditional_execution(work_data, loop_bound, use_target, current_seed);
        
        for (int i = 0; i < loop_bound; i += 4) {
            checksum += work_data[i];
        }
        free(work_data);
        
        /* Modify arrays for next iteration */
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            static_array[i] = (static_array[i] + auto_array[i]) % 1000;
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Seed used: %d\n", seed);
    printf("SIMT variant enabled: %s\n", use_simt_variant ? "yes" : "no");
    
    return 0;
}
