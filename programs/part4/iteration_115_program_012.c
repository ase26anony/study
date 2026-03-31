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
            int team = omp_get_team_num();
            
            /* Complex condition to force conditional code generation */
            if ((tid % 3 == 0) && (i % 2 == 0) && (team % 2 == 0)) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((tid % 5 == 0) && (j % 3 == 0)) {
                A[idx] = B[idx] - C[idx] * iter;
            } else {
                A[idx] = (B[idx] + C[idx]) * (iter % 7);
            }
            
            /* Additional control flow with early exit simulation */
            if (A[idx] > 1000 && tid % 4 == 0) {
                A[idx] = A[idx] % 100;
            }
        }
    }
}

/* Function 2: Mapped data with pointer-based accesses and SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32) aligned(X:64, Y:64) num_teams(size/64)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern - harder to optimize */
        int idx = indices[i % size];
        
        /* SIMD-friendly but with conditional */
        if (idx >= 0 && idx < size) {
            float temp = Y[idx];
            
            /* Branch that depends on value */
            if (temp > 0.5f) {
                X[i] = temp * temp + sinf((float)i * 0.01f);
            } else {
                X[i] = temp * 0.5f + cosf((float)i * 0.01f);
            }
            
            /* Additional computation with potential divergence */
            if (i % 8 == 0) {
                X[i] = X[i] * 2.0f - 1.0f;
            }
        } else {
            X[i] = 0.0f;
        }
    }
}

/* Function 3: Target region with nested parallel for simd */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int offset) {
    #pragma omp target map(to: rows, cols, offset) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(rows) thread_limit(128)
    {
        #pragma omp parallel for simd collapse(2) reduction(+:offset)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                int tid = omp_get_thread_num();
                
                /* Complex condition depending on thread ID and mask */
                if ((tid % 2 == 0) && (mask[idx] > 0)) {
                    D[idx] = D[idx] * 1.5 + (double)(tid % 16);
                } else if ((tid % 3 == 0) && (mask[idx] < 0)) {
                    D[idx] = D[idx] * 0.5 - (double)(tid % 8);
                } else {
                    D[idx] = D[idx] + (double)((i + j + offset) % 32);
                }
                
                /* Nested condition */
                if (D[idx] > 100.0 && tid % 4 == 0) {
                    D[idx] = fmod(D[idx], 50.0);
                }
            }
        }
    }
}

/* Function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int size, int scale) {
    /* First a simple target teams distribute */
    #pragma omp target teams distribute map(to: size, scale) map(tofrom: data[0:size]) \
        num_teams(size/32)
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * scale;
    }
    
    /* Then a parallel for simd inside the same function */
    #pragma omp target teams distribute parallel for simd \
        map(to: size) map(tofrom: data[0:size]) \
        num_teams(size/64) thread_limit(128)
    for (int i = 0; i < size; i++) {
        if (i % 2 == 0) {
            data[i] = data[i] + omp_get_thread_num();
        } else {
            data[i] = data[i] - omp_get_team_num();
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
    int n = 512;
    int m = 256;
    int iterations = 10;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    printf("Running with n=%d, m=%d, iterations=%d\n", n, m, iterations);
    
    /* Calculate sizes */
    int total_size = n * m;
    size_t int_size = total_size * sizeof(int);
    size_t float_size = total_size * sizeof(float);
    size_t double_size = total_size * sizeof(double);
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(int_size);
    int *B = (int *)malloc(int_size);
    int *C = (int *)malloc(int_size);
    int *mask = (int *)malloc(int_size);
    
    float *X = (float *)malloc(float_size);
    float *Y = (float *)malloc(float_size);
    
    double *D = (double *)malloc(double_size);
    
    int *indices = (int *)malloc(int_size);
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        mask[i] = (i % 5 == 0) ? 1 : ((i % 3 == 0) ? -1 : 0);
        indices[i] = (i * 7) % total_size;
        
        X[i] = (float)(i % 101) / 100.0f;
        Y[i] = (float)((i + 23) % 103) / 100.0f;
        
        D[i] = (double)(i % 107) / 10.0;
    }
    
    unsigned long checksum_before = 0;
    checksum_before += compute_checksum(A, int_size);
    checksum_before += compute_checksum(X, float_size);
    checksum_before += compute_checksum(D, double_size);
    
    printf("Checksum before: %lu\n", checksum_before);
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        test_simt_nested(A, B, C, n, m, iter);
        
        if (iter % 2 == 0) {
            test_simt_mapped(X, Y, indices, total_size, 1);
        } else {
            test_simt_mapped(X, Y, indices, total_size, 2);
        }
        
        test_simt_conditional(D, mask, n, m, iter);
        
        if (iter % 3 == 0) {
            test_mixed_constructs(C, total_size, iter + 1);
        }
    }
    
    /* Compute final checksum */
    unsigned long checksum_after = 0;
    checksum_after += compute_checksum(A, int_size);
    checksum_after += compute_checksum(X, float_size);
    checksum_after += compute_checksum(D, double_size);
    checksum_after += compute_checksum(C, int_size);
    
    printf("Checksum after: %lu\n", checksum_after);
    printf("Difference: %ld\n", checksum_after - checksum_before);
    
    /* Cleanup */
    free(A);
    free(B);
    free(C);
    free(mask);
    free(X);
    free(Y);
    free(D);
    free(indices);
    
    return 0;
}
