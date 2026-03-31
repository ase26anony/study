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
                      int start, int end, int stride, volatile int n) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int local_private = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) num_teams(4) thread_limit(128) \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        private(local_private) firstprivate(chunk_size) \
        shared(static_counter)
    for (int i = 0; i < n; i += chunk_size) {
        for (int j = 0; j < chunk_size && (i + j) < n; j++) {
            int idx = i + j;
            if (idx >= start && idx < end && (idx - start) % stride == 0) {
                c[idx] = a[idx] * b[idx] + local_private;
            }
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float * restrict x, float * restrict y, 
                          float alpha, volatile int m, int offset) {
    float * restrict z = (float *)malloc(m * sizeof(float));
    if (!z) return;
    
    #pragma omp target data map(to: x[offset:m], y[offset:m]) \
                            map(from: z[offset:m])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(8) thread_limit(64) \
            firstprivate(alpha, offset) reduction(+:z[offset:m])
        for (int i = offset; i < offset + m; i++) {
            z[i] = x[i] * alpha + y[i];
            /* Complex indexing to prevent optimization */
            z[i] += (i % 3 == 0) ? x[i/2] : y[i*2 % m];
        }
    }
    
    /* Use results to prevent elimination */
    float sum = 0.0f;
    for (int i = 0; i < m; i++) {
        sum += z[i];
    }
    printf("Parallel loop checksum: %f\n", sum);
    
    free(z);
}

/* Variant 3: Combined constructs with pointer arithmetic */
void combined_constructs(double * restrict d1, double * restrict d2,
                         int size, volatile int iter, int * restrict mask) {
    double *ptr1 = d1;
    double *ptr2 = d2 + size/2;
    const double scale = 2.5;
    
    #pragma omp target data map(to: ptr1[0:size/2], mask[0:size]) \
                            map(tofrom: ptr2[-size/4:size/2])
    {
        #pragma omp target teams distribute parallel for simd \
            collapse(2) schedule(static, 16) \
            firstprivate(scale, iter) private(ptr1, ptr2)
        for (int i = 0; i < size/2; i += 32) {
            for (int j = 0; j < 32 && (i + j) < size/2; j++) {
                int idx = i + j;
                if (mask[idx]) {
                    ptr2[idx - size/4] = ptr1[idx] * scale * iter 
                                       + ptr2[idx - size/4];
                } else {
                    ptr2[idx - size/4] = ptr1[idx] / scale - iter;
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int n, volatile int seed) {
    int local_seed = seed;
    #pragma omp parallel for simd schedule(dynamic) \
        firstprivate(local_seed) reduction(+:arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = (arr[i] * local_seed + i) % 1000;
    }
}

/* Function that selects between target and host execution */
void conditional_execution(int *data, int size, int use_target, 
                           volatile int threshold) {
    if (use_target && threshold > 0) {
        /* This may trigger SIMT transformation */
        int *temp = (int *)malloc(size * sizeof(int));
        memcpy(temp, data, size * sizeof(int));
        
        #pragma omp target teams distribute parallel for simd \
            map(to: temp[0:size]) map(from: data[0:size]) \
            if(target: threshold > 50)
        for (int i = 0; i < size; i++) {
            data[i] = temp[i] * threshold + i;
        }
        
        free(temp);
    } else {
        /* Host-only path */
        host_only_parallel(data, size, threshold);
    }
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Declare arrays with different storage durations */
    static int static_array[N];
    int auto_array[N];
    const int const_size = M;
    volatile int vol_bound = N;
    
    float float_array[M];
    double double_array[N];
    int mask_array[N];
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        double_array[i] = (double)rand() / RAND_MAX;
        mask_array[i] = rand() % 2;
    }
    
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)rand() / RAND_MAX;
    }
    
    printf("Starting OpenMP SIMT transformation test...\n");
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        volatile int dynamic_bound = N - iter * 100;
        int use_target = (rand() % 2) && (iter % 2 == 0);
        int offset = rand() % 100;
        int stride = 1 + rand() % 3;
        
        printf("\nIteration %d: bound=%d, target=%d\n", 
               iter, dynamic_bound, use_target);
        
        /* Call variant functions with different parameters */
        if (use_target) {
            simd_target_loop(static_array, auto_array, auto_array,
                            offset, dynamic_bound, stride, vol_bound);
        }
        
        parallel_target_loop(float_array, float_array, 
                           (float)iter, const_size, offset);
        
        combined_constructs(double_array, double_array,
                          N, iter, mask_array);
        
        /* Conditional execution that may trigger SIMT */
        conditional_execution(auto_array, N, use_target, iter * 20);
        
        /* Compute checksums to prevent elimination */
        int int_sum = 0;
        float float_sum = 0.0f;
        double double_sum = 0.0;
        
        for (int i = 0; i < N; i++) {
            int_sum += auto_array[i];
            double_sum += double_array[i];
        }
        
        for (int i = 0; i < M; i++) {
            float_sum += float_array[i];
        }
        
        printf("Checksums - int: %d, float: %f, double: %lf\n",
               int_sum, float_sum, double_sum);
    }
    
    /* Final verification */
    printf("\nFinal verification...\n");
    int final_check = 0;
    #pragma omp parallel for simd reduction(+:final_check) \
        if(parallel: seed > 10000)
    for (int i = 0; i < N; i++) {
        final_check += static_array[i] + auto_array[i];
    }
    printf("Final check: %d\n", final_check);
    
    return 0;
}
