#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define M 512
#define MAX_ITER 5

/* Variant 1: SIMD target loop with complex data environment */
void simd_target_loop(int *a, int *b, int *c, int start, int end, int step, 
                      volatile int bound, int use_simd) {
    static int static_counter = 0;
    const int chunk_size = 64;
    int local_private = static_counter++;
    
    /* Mixed storage duration and qualifiers */
    static float static_data[N];
    float auto_data[M];
    const float pi = 3.14159f;
    
    /* Initialize some data */
    #pragma omp parallel for simd
    for (int i = 0; i < M; i++) {
        auto_data[i] = i * 0.5f;
    }
    
    /* Main target region with SIMD clause */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[start:end:step], b[start:end:step]) \
        map(from: c[start:end:step]) \
        firstprivate(local_private, chunk_size) \
        shared(static_data) \
        private(auto_data) \
        num_teams(bound % 8 + 1) \
        thread_limit(256)
    for (int i = start; i < end; i += step) {
        int idx = i;
        if (use_simd) {
            /* Complex indexing to prevent optimization */
            int j = (idx * 31 + 17) % (end - start);
            c[idx] = a[idx] + b[j] + local_private + (int)(auto_data[idx % M] * pi);
        } else {
            c[idx] = a[idx] * b[idx] - local_private;
        }
        
        /* Side effect to prevent dead code elimination */
        static_data[idx % N] += c[idx] * 0.001f;
    }
}

/* Variant 2: Target loop without explicit SIMD clause */
void parallel_target_loop(float *x, float *y, float *z, int rows, int cols,
                         volatile int seed, int use_collapse) {
    const float alpha = 1.5f;
    float beta = 2.0f;
    
    /* Pointer arithmetic and array sections */
    float *x_slice = x + (seed % 32);
    float *y_slice = y + (seed % 32);
    float *z_slice = z + (seed % 32);
    int slice_len = rows * cols - (seed % 32);
    
    if (use_collapse) {
        #pragma omp target teams distribute parallel for collapse(2) \
            map(to: x_slice[0:slice_len], y_slice[0:slice_len]) \
            map(from: z_slice[0:slice_len]) \
            firstprivate(alpha, beta) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                if (idx < slice_len) {
                    /* Runtime-dependent computation */
                    float t = (seed % 2) ? x_slice[idx] : y_slice[idx];
                    z_slice[idx] = alpha * x_slice[idx] + beta * y_slice[idx] + 
                                  t * (i + j) * 0.01f;
                }
            }
        }
    } else {
        #pragma omp target teams distribute parallel for \
            map(to: x_slice[0:slice_len], y_slice[0:slice_len]) \
            map(from: z_slice[0:slice_len])
        for (int i = 0; i < slice_len; i++) {
            z_slice[i] = x_slice[i] * y_slice[i] / (alpha + beta);
        }
    }
}

/* Variant 3: Combined constructs with nested data regions */
void combined_constructs(double *mat1, double *mat2, double *result,
                        int dim1, int dim2, int dim3, volatile int flag) {
    /* Separate data region */
    #pragma omp target data map(to: mat1[0:dim1*dim2], mat2[0:dim2*dim3]) \
                            map(from: result[0:dim1*dim3])
    {
        /* Nested loops with runtime bounds */
        int block_size = (flag % 16) + 8;
        
        #pragma omp target teams distribute parallel for simd \
            collapse(2) firstprivate(block_size) \
            num_teams((dim1 + block_size - 1) / block_size) \
            thread_limit(64)
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim3; j++) {
                double sum = 0.0;
                int limit = (flag % 2) ? dim2 : (dim2 / 2);
                
                /* Inner loop that might get SIMT transformed */
                #pragma omp simd reduction(+:sum)
                for (int k = 0; k < limit; k++) {
                    sum += mat1[i * dim2 + k] * mat2[k * dim3 + j];
                }
                
                result[i * dim3 + j] = sum + (i * j * 0.01);
            }
        }
    }
}

/* Host-only parallel region for conditional execution */
void host_only_parallel(int *data, int size, int factor) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * factor + omp_get_thread_num();
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    printf("Seed: %d\n", seed);
    
    /* Volatile variables to prevent constant folding */
    volatile int v_bound = rand() % 100 + 50;
    volatile int v_flag = rand() % 10;
    volatile int v_size = N;
    
    /* Arrays with different types and storage */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    
    float *x = (float*)malloc(M * M * sizeof(float));
    float *y = (float*)malloc(M * M * sizeof(float));
    float *z = (float*)malloc(M * M * sizeof(float));
    
    double *mat1 = (double*)malloc(64 * 128 * sizeof(double));
    double *mat2 = (double*)malloc(128 * 256 * sizeof(double));
    double *result = (double*)malloc(64 * 256 * sizeof(double));
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = 0;
    }
    
    for (int i = 0; i < M * M; i++) {
        x[i] = (float)rand() / RAND_MAX;
        y[i] = (float)rand() / RAND_MAX;
        z[i] = 0.0f;
    }
    
    for (int i = 0; i < 64 * 128; i++) {
        mat1[i] = (double)rand() / RAND_MAX;
    }
    for (int i = 0; i < 128 * 256; i++) {
        mat2[i] = (double)rand() / RAND_MAX;
    }
    
    /* Main loop with varying parameters */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        printf("\nIteration %d:\n", iter);
        
        /* Runtime condition to choose between target and host */
        int use_target = (rand() % 3) > 0;
        int use_simd = (rand() % 2);
        int use_collapse = (rand() % 2);
        
        if (use_target) {
            /* Call target variants with varying parameters */
            int start = rand() % 100;
            int end = N - (rand() % 100);
            int step = (rand() % 3) + 1;
            
            simd_target_loop(a, b, c, start, end, step, v_bound + iter, use_simd);
            
            /* Verify results */
            int checksum = 0;
            for (int i = start; i < end; i += step) {
                checksum += c[i];
            }
            printf("  SIMD target checksum: %d\n", checksum);
            
            /* Second variant */
            int rows = (rand() % 32) + 16;
            int cols = (rand() % 32) + 16;
            
            parallel_target_loop(x, y, z, rows, cols, v_flag + iter, use_collapse);
            
            /* Verify results */
            float fchecksum = 0.0f;
            for (int i = 0; i < rows * cols; i++) {
                fchecksum += z[i];
            }
            printf("  Parallel target checksum: %.2f\n", fchecksum);
            
            /* Third variant */
            combined_constructs(mat1, mat2, result, 64, 128, 256, v_flag * iter);
            
            /* Verify results */
            double dchecksum = 0.0;
            for (int i = 0; i < 64 * 256; i += 257) { /* Strided access */
                dchecksum += result[i];
            }
            printf("  Combined constructs checksum: %.2f\n", dchecksum);
        } else {
            /* Host-only path */
            host_only_parallel(a, v_size, iter + 1);
            
            int checksum = 0;
            for (int i = 0; i < v_size; i += 13) {
                checksum += a[i];
            }
            printf("  Host-only checksum: %d\n", checksum);
        }
        
        /* Modify volatile variable for next iteration */
        v_bound += rand() % 10;
        v_flag ^= (iter << 3);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(x);
    free(y);
    free(z);
    free(mat1);
    free(mat2);
    free(result);
    
    return 0;
}
