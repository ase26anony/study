#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 5381

/* Function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m]) \
        num_teams(n/32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            int tid = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            /* Complex condition to prevent optimization */
            if ((tid % 3 == 0) && (team_id % 2 == 0) && (i > j) && (iter % 2 == 0)) {
                A[idx] = B[idx] * C[idx] + tid;
            } else if ((tid % 5 == 0) && (i < j) && (iter % 3 == 0)) {
                A[idx] = B[idx] - C[idx] + team_id;
            } else {
                A[idx] = B[idx] + C[idx] + (tid ^ team_id);
            }
            
            /* Additional control flow to complicate SIMD transformation */
            if (A[idx] % 7 == 0) {
                A[idx] = A[idx] * 2;
            } else if (A[idx] % 11 == 0) {
                A[idx] = A[idx] / 3;
            }
        }
    }
}

/* Function 2: Mapped data with pointer-based accesses and SIMD safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32) aligned(X:64, Y:64) num_teams(size/128)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern - harder to vectorize */
        int idx = indices[i % size];
        
        /* Conditional store with pointer arithmetic */
        float *ptr = &X[i];
        float *src_ptr = &Y[idx];
        
        if (ptr != NULL && src_ptr != NULL) {
            /* Complex computation with conditionals */
            if (i % 4 == 0) {
                *ptr = *src_ptr * 2.0f + (float)(i % 8);
            } else if (i % 4 == 1) {
                *ptr = *src_ptr / 1.5f - (float)(i % 16);
            } else if (i % 4 == 2) {
                *ptr = sqrtf(fabsf(*src_ptr)) + (float)omp_get_thread_num();
            } else {
                *ptr = *src_ptr * *src_ptr - (float)omp_get_team_num();
            }
            
            /* Additional SIMD-unfriendly control flow */
            if (*ptr < 0.0f) {
                *ptr = -*ptr;
            }
        }
    }
}

/* Function 3: Target with nested parallel for simd and thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int offset) {
    #pragma omp target map(to: rows, cols, offset) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp teams distribute
        for (int i = 0; i < rows; i++) {
            #pragma omp parallel for simd
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Thread-dependent condition - crucial for SIMT transformation */
                int tid = omp_get_thread_num();
                int lane = tid % 32;  /* Simulate warp lane */
                
                if (lane % 2 == 0) {
                    /* Even lanes do one computation */
                    D[idx] = sin((double)idx * 0.1) * (double)mask[idx];
                } else {
                    /* Odd lanes do different computation */
                    D[idx] = cos((double)idx * 0.1) / ((double)mask[idx] + 1.0);
                }
                
                /* Additional condition based on thread hierarchy */
                if (omp_get_team_num() % 2 == 0) {
                    D[idx] += (double)offset;
                } else {
                    D[idx] -= (double)offset;
                }
                
                /* SIMD divergence within warp */
                if (D[idx] > 100.0) {
                    D[idx] = 100.0;
                } else if (D[idx] < -100.0) {
                    D[idx] = -100.0;
                }
            }
        }
    }
}

/* Function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int size, int scale) {
    /* First target region: teams distribute simd */
    #pragma omp target teams distribute simd \
        map(to: size, scale) map(tofrom: data[0:size]) \
        num_teams(size/64) thread_limit(128)
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * scale + omp_get_team_num();
    }
    
    /* Second target region: parallel for simd */
    #pragma omp target parallel for simd \
        map(to: size) map(tofrom: data[0:size])
    for (int i = 0; i < size; i++) {
        if (omp_get_thread_num() % 4 == 0) {
            data[i] = data[i] << 2;
        } else {
            data[i] = data[i] >> 1;
        }
    }
}

/* Compute checksum to verify execution */
unsigned long compute_checksum(void *data, size_t size) {
    unsigned long checksum = CHECKSUM_SEED;
    unsigned char *bytes = (unsigned char *)data;
    
    for (size_t i = 0; i < size; i++) {
        checksum = ((checksum << 5) + checksum) + bytes[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int n = 1000, m = 200, iterations = 5;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    printf("Running with n=%d, m=%d, iterations=%d\n", n, m, iterations);
    
    /* Dynamic allocation with non-constant sizes */
    int total_size = n * m;
    size_t int_size = total_size * sizeof(int);
    size_t float_size = total_size * sizeof(float);
    size_t double_size = total_size * sizeof(double);
    
    int *A = (int *)malloc(int_size);
    int *B = (int *)malloc(int_size);
    int *C = (int *)malloc(int_size);
    int *indices = (int *)malloc(int_size);
    int *mask = (int *)malloc(int_size);
    float *X = (float *)malloc(float_size);
    float *Y = (float *)malloc(float_size);
    double *D = (double *)malloc(double_size);
    
    if (!A || !B || !C || !indices || !mask || !X || !Y || !D) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern-based data */
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        indices[i] = (i * 7) % total_size;
        mask[i] = (i % 19) + 1;
        X[i] = (float)(i % 255) * 0.1f;
        Y[i] = (float)((i + 17) % 255) * 0.2f;
        D[i] = (double)(i % 100) * 0.01;
    }
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        /* Call functions with varying parameters to trigger different paths */
        test_simt_nested(A, B, C, n, m, iter);
        
        int stride = (iter % 4) + 1;
        test_simt_mapped(X, Y, indices, total_size, stride);
        
        int offset = iter * 10;
        test_simt_conditional(D, mask, n, m, offset);
        
        /* Test with different sizes each iteration */
        int current_size = total_size / ((iter % 3) + 1);
        test_mixed_constructs(A, current_size, iter + 1);
        
        /* Force synchronization between iterations */
        #pragma omp target update from(A[0:total_size], X[0:total_size], D[0:total_size])
    }
    
    /* Compute and print checksums to verify execution */
    unsigned long checksum_A = compute_checksum(A, int_size);
    unsigned long checksum_X = compute_checksum(X, float_size);
    unsigned long checksum_D = compute_checksum(D, double_size);
    
    printf("Checksums:\n");
    printf("  Array A: %lu\n", checksum_A);
    printf("  Array X: %lu\n", checksum_X);
    printf("  Array D: %lu\n", checksum_D);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices); free(mask);
    free(X); free(Y); free(D);
    
    return 0;
}
