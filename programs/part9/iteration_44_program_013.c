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
                      volatile int bound, int iter) {
    static int static_counter = 0;
    const int chunk_size = 32;
    int local_private = iter;
    
    /* Mixed storage duration and complex mapping */
    #pragma omp target teams distribute parallel for simd \
        collapse(2) \
        map(to: a[start:end:stride], b[start:end:stride]) \
        map(from: c[start:end:stride]) \
        private(local_private) \
        firstprivate(static_counter) \
        shared(chunk_size) \
        num_teams(bound % 8 + 1) \
        thread_limit(256)
    for (int i = start; i < end; i += stride) {
        for (int j = 0; j < chunk_size; j++) {
            /* Complex index calculation to prevent optimization */
            int idx = (i * 7919 + j * 65537) % (end - start);
            if (idx < 0) idx = -idx;
            
            /* Runtime-dependent computation */
            c[i] = a[i] * (bound % 7 + 1) + b[i] / (local_private + 1);
            c[i] += (i % 3 == 0) ? static_counter : -static_counter;
            
            /* Pointer arithmetic */
            int *ptr = c + i;
            *ptr += (j % 2) ? idx : -idx;
        }
    }
    
    static_counter++;
}

/* Variant 2: Parallel target loop without SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, 
                         int low, int high, int step,
                         volatile float scale) {
    const float pi = 3.14159265f;
    float firstprivate_val = scale;
    
    /* Array section with stride in map clause */
    #pragma omp target teams distribute parallel for \
        map(to: x[low:high:step], y[low:high:step]) \
        map(tofrom: z[low:high:step]) \
        firstprivate(firstprivate_val, pi) \
        reduction(+:firstprivate_val)
    for (int i = low; i < high; i += step) {
        /* Trigonometric computation to create complex data flow */
        float angle = (float)i * pi / (high - low);
        z[i] = x[i] * __builtin_sinf(angle) + y[i] * __builtin_cosf(angle);
        
        /* Conditional execution path */
        if (i % 4 == 0) {
            z[i] *= firstprivate_val;
            firstprivate_val += 0.1f;
        } else if (i % 4 == 1) {
            z[i] /= (firstprivate_val + 1.0f);
        }
        
        /* Array access with pointer arithmetic */
        float *base = &z[low];
        float offset = (float)(i - low) / (high - low);
        base[(i - low) % step] += offset * scale;
    }
}

/* Variant 3: Combined constructs with nested data regions */
void combined_constructs(double *mat1, double *mat2, double *result,
                        int rows, int cols, int tile,
                        volatile int use_simd) {
    /* Static array to create bind expressions */
    static double cache[256];
    const int cache_size = 256;
    
    /* Target data region followed by compute region */
    #pragma omp target data \
        map(to: mat1[0:rows*cols], mat2[0:rows*cols]) \
        map(alloc: cache[0:cache_size]) \
        map(from: result[0:rows*cols])
    {
        /* Initialize cache with data-dependent values */
        #pragma omp target teams distribute parallel for simd \
            if(use_simd > 0) \
            num_teams(rows/tile + 1) \
            thread_limit(128)
        for (int i = 0; i < cache_size; i++) {
            cache[i] = (double)((i * 13) % 17) / 17.0;
        }
        
        /* Nested loops with collapse */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) \
            map(always, tofrom: cache[0:cache_size]) \
            private(tile)
        for (int i = 0; i < rows; i += tile) {
            for (int j = 0; j < cols; j += tile) {
                int tile_end_i = (i + tile < rows) ? i + tile : rows;
                int tile_end_j = (j + tile < cols) ? j + tile : cols;
                
                /* Tiled matrix operation */
                for (int ti = i; ti < tile_end_i; ti++) {
                    for (int tj = j; tj < tile_end_j; tj++) {
                        int idx = ti * cols + tj;
                        int cache_idx = (ti % 16) * 16 + (tj % 16);
                        result[idx] = mat1[idx] * mat2[idx] + cache[cache_idx];
                        
                        /* Update cache with result */
                        cache[cache_idx] = result[idx] * 0.99;
                    }
                }
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *arr, int size, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * factor) % 1001;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Volatile variables to prevent constant folding */
    volatile int v_bound = rand() % 100 + 50;
    volatile float v_scale = (float)(rand() % 100) / 10.0f;
    volatile int v_use_simd = rand() % 2;
    
    /* Arrays with different types and storage */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    
    float *x = (float*)malloc(M * sizeof(float));
    float *y = (float*)malloc(M * sizeof(float));
    float *z = (float*)malloc(M * sizeof(float));
    
    double *mat1 = (double*)malloc(N * M * sizeof(double));
    double *mat2 = (double*)malloc(N * M * sizeof(double));
    double *result = (double*)malloc(N * M * sizeof(double));
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = 0;
    }
    
    for (int i = 0; i < M; i++) {
        x[i] = (float)(rand() % 1000) / 10.0f;
        y[i] = (float)(rand() % 1000) / 10.0f;
        z[i] = 0.0f;
    }
    
    for (int i = 0; i < N * M; i++) {
        mat1[i] = (double)(rand() % 1000) / 100.0;
        mat2[i] = (double)(rand() % 1000) / 100.0;
        result[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMT transformation test (seed: %d)\n", seed);
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        /* Vary parameters each iteration */
        int start = rand() % 100;
        int end = N - rand() % 100;
        int stride = (rand() % 5) + 1;
        
        int low = rand() % 50;
        int high = M - rand() % 50;
        int step = (rand() % 3) + 1;
        
        int tile = 8 + rand() % 16;
        
        /* Conditional execution path */
        if (v_use_simd || (iter % 3 == 0)) {
            printf("Iteration %d: Calling SIMD target function\n", iter);
            simd_target_loop(a, b, c, start, end, stride, v_bound, iter);
            
            /* Verify results with checksum */
            long long checksum = 0;
            for (int i = start; i < end; i += stride) {
                checksum += c[i];
            }
            printf("  SIMD checksum: %lld\n", checksum);
        } else {
            printf("Iteration %d: Calling host-only parallel function\n", iter);
            host_only_parallel(a, N, iter + 1);
        }
        
        /* Always call parallel target variant */
        printf("Iteration %d: Calling parallel target function\n", iter);
        parallel_target_loop(x, y, z, low, high, step, v_scale);
        
        /* Verify results */
        float fchecksum = 0.0f;
        for (int i = low; i < high; i += step) {
            fchecksum += z[i];
        }
        printf("  Parallel checksum: %.2f\n", fchecksum);
        
        /* Call combined constructs with probability */
        if (iter % 2 == 0 || v_use_simd) {
            printf("Iteration %d: Calling combined constructs function\n", iter);
            combined_constructs(mat1, mat2, result, N, M, tile, v_use_simd);
            
            /* Verify results */
            double dchecksum = 0.0;
            for (int i = 0; i < N * M; i += 97) {
                dchecksum += result[i];
            }
            printf("  Combined checksum: %.2f\n", dchecksum);
        }
        
        /* Update volatile variables for next iteration */
        v_bound = (v_bound * 13 + 7) % 200;
        v_scale += 0.5f;
        v_use_simd = !v_use_simd;
    }
    
    /* Final verification */
    printf("\nFinal verification:\n");
    
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i];
    }
    printf("Integer arrays sum: %d\n", final_sum);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat1); free(mat2); free(result);
    
    return 0;
}
