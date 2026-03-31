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
    int local_private = static_counter++;
    
    #pragma omp target teams distribute parallel for simd \
        collapse(2) private(local_private) firstprivate(chunk_size) \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        num_teams(n/64) thread_limit(128)
    for (int i = 0; i < n; i += chunk_size) {
        for (int j = 0; j < chunk_size && (i + j) < n; j++) {
            int idx = i + j;
            /* Complex indexing to prevent optimization */
            int offset = (idx % 2 == 0) ? idx : (idx * 3) % n;
            c[offset] = a[offset] + b[offset] + local_private;
        }
    }
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *restrict x, float *restrict y, 
                         float scale, volatile int m, int offset) {
    float local_scale = scale;
    int *dynamic_array = (int*)malloc(m * sizeof(int));
    
    #pragma omp target data map(to: x[offset:m/2]) map(from: y[offset:m/2]) \
        map(alloc: dynamic_array[0:m])
    {
        #pragma omp target teams distribute parallel for \
            firstprivate(local_scale) shared(dynamic_array) \
            num_teams(m/32)
        for (int i = offset; i < offset + m/2; i++) {
            dynamic_array[i - offset] = i;
            y[i] = x[i] * local_scale + (float)dynamic_array[i - offset];
        }
    }
    
    free(dynamic_array);
}

/* Variant 3: Combined constructs with pointer arithmetic */
void combined_constructs(double *restrict arr1, double *restrict arr2,
                        double *restrict result, int size, 
                        volatile int iter, int *mask) {
    double *ptr1 = arr1;
    double *ptr2 = arr2 + size/2;
    const double pi = 3.141592653589793;
    
    #pragma omp target data map(to: ptr1[0:size/2], ptr2[-size/2:size/2]) \
        map(tofrom: result[0:size]) map(to: mask[0:size])
    {
        #pragma omp target teams distribute parallel for simd \
            collapse(2) firstprivate(pi, iter)
        for (int i = 0; i < size; i += 32) {
            for (int j = 0; j < 32 && (i + j) < size; j++) {
                int idx = i + j;
                if (mask[idx]) {
                    /* Complex computation with branches */
                    result[idx] = (ptr1[idx] * ptr2[idx]) / 
                                 (pi * (iter + 1)) + 
                                 sin((double)idx * pi / size);
                } else {
                    result[idx] = ptr1[idx] + ptr2[idx];
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *data, int n, volatile int seed) {
    #pragma omp parallel for simd schedule(static, 8)
    for (int i = 0; i < n; i++) {
        data[i] = (data[i] * seed) % 1000;
    }
}

/* Function to select between target and host execution */
void conditional_execution(int *a, int *b, int *c, int n, 
                          volatile int choice, volatile int threshold) {
    if (choice % 2 == 0 && threshold > 100) {
        /* This path may trigger SIMT transformation */
        simd_target_loop(a, b, c, 0, n, 1, n);
    } else {
        /* Host-only path */
        host_only_parallel(a, n, choice);
        #pragma omp parallel for simd
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Volatile variables to prevent constant folding */
    volatile int v_size = N + (rand() % 256);
    volatile int v_iter = MAX_ITER;
    volatile int threshold = rand() % 200;
    
    /* Arrays with different storage durations */
    static int static_array[N];
    int auto_array[N];
    float float_array[M];
    double double_array[N];
    int *heap_array = (int*)malloc(N * sizeof(int));
    int *heap_array2 = (int*)malloc(N * sizeof(int));
    int *result_array = (int*)malloc(N * sizeof(int));
    float *float_result = (float*)malloc(M * sizeof(float));
    double *double_result = (double*)malloc(N * sizeof(double));
    int *mask = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < N; i++) {
        static_array[i] = rand() % 100;
        auto_array[i] = rand() % 100;
        heap_array[i] = rand() % 100;
        heap_array2[i] = rand() % 100;
        double_array[i] = (double)rand() / RAND_MAX;
        mask[i] = rand() % 2;
    }
    
    for (int i = 0; i < M; i++) {
        float_array[i] = (float)rand() / RAND_MAX;
    }
    
    int checksum = 0;
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < v_iter; iter++) {
        volatile int choice = rand() % 10;
        volatile int offset = rand() % 128;
        volatile int stride = 1 + (rand() % 4);
        
        /* Call different variants to expose multiple contexts */
        switch (iter % 4) {
            case 0:
                simd_target_loop(heap_array, heap_array2, result_array,
                                offset, v_size, stride, v_size);
                break;
            case 1:
                parallel_target_loop(float_array, float_result,
                                    (float)(iter + 1), M, offset);
                break;
            case 2:
                combined_constructs(double_array, double_array,
                                   double_result, N, iter, mask);
                break;
            case 3:
                conditional_execution(auto_array, static_array,
                                     result_array, N, choice, threshold);
                break;
        }
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += result_array[i] + auto_array[i];
        }
        for (int i = 0; i < M; i++) {
            checksum += (int)float_result[i];
        }
        
        /* Modify data for next iteration */
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            heap_array[i] = (heap_array[i] + iter) % 1000;
            mask[i] = (mask[i] + i) % 2;
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Seed used: %d\n", seed);
    
    /* Cleanup */
    free(heap_array);
    free(heap_array2);
    free(result_array);
    free(float_result);
    free(double_result);
    free(mask);
    
    return 0;
}
