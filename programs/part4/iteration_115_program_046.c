#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 100000

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop variables
               This should trigger the SIMT transformation with conditional labels */
            if ((omp_get_thread_num() % 4) == 0) {
                A[idx] = B[idx] * 2;
            } else if ((i + j) % 3 == 0) {
                A[idx] = B[idx] + C[idx];
            } else {
                A[idx] = B[idx] - C[idx];
            }
            
            /* Additional control flow to complicate SIMD lane divergence */
            if (A[idx] > 1000) {
                A[idx] = A[idx] % 1000;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD clause */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, Y[0:size*2], indices[0:size]) \
        map(tofrom: X[0:size]) \
        safelen(32)
    for (int i = 0; i < size; i++) {
        /* Indirect access pattern - may influence SIMT memory coalescing decisions */
        int idx = indices[i] % size;
        
        /* Complex computation with conditional */
        if (i % stride == 0) {
            X[i] = Y[idx] * 2.0f + Y[idx + size] * 0.5f;
        } else {
            X[i] = sqrtf(fabsf(Y[idx])) + (float)(i % 8);
        }
        
        /* Nested condition to increase control flow complexity */
        if (X[i] < 0.0f) {
            X[i] = -X[i];
        }
    }
}

/* Test 3: Multiple SIMD loops with different characteristics */
void test_simt_conditional(double *D, int *mask, int n, int m) {
    #pragma omp target teams distribute parallel for simd \
        map(to: n, m, mask[0:n]) map(tofrom: D[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Thread-dependent condition - forces SIMT divergence handling */
            int thread_mod = omp_get_thread_num() % 8;
            
            switch (thread_mod) {
                case 0:
                case 1:
                    D[idx] = (double)(i * j) * 1.5;
                    break;
                case 2:
                case 3:
                    D[idx] = (double)(i + j) / (double)(mask[i] + 1);
                    break;
                default:
                    D[idx] = sin((double)idx * 0.01) * 100.0;
                    break;
            }
            
            /* Additional SIMD-unfriendly operation */
            if (mask[i] > 0 && D[idx] > 100.0) {
                D[idx] = log(D[idx] + 1.0);
            }
        }
    }
}

/* Test 4: Mixed parallel constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int size, int iter) {
    /* First a simple target region */
    #pragma omp target teams distribute parallel for \
        map(to: size, iter) map(tofrom: data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = i % 256;
    }
    
    /* Then a SIMD-focused region */
    #pragma omp target teams distribute parallel for simd \
        map(to: size) map(tofrom: data[0:size]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < size; i++) {
        /* Complex conditional that depends on previous computation */
        if (data[i] > 128) {
            data[i] = data[i] * 2 - 255;
        } else {
            data[i] = (data[i] * 3) % 256;
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *C, float *X, float *Y, 
                 double *D, int *indices, int *mask, int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        X[i] = (float)(i % 100) * 0.1f;
        Y[i] = (float)(i % 200) * 0.05f;
        D[i] = (double)(i % 300) * 0.01;
        indices[i] = (i * 7) % total_size;
        mask[i] = (i % 11) - 5;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int total_size) {
    long long checksum = 0;
    
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < total_size; i++) {
        checksum += (long long)A[i];
        checksum += (long long)(X[i] * 100.0f);
        checksum += (long long)(D[i] * 1000.0);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int n = 512;
    int m = 256;
    
    /* Parse command line arguments for flexibility */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    if (n <= 0) n = 512;
    if (m <= 0) m = 256;
    
    int total_size = n * m;
    
    printf("Testing SIMT transformation with n=%d, m=%d, total_size=%d\n", 
           n, m, total_size);
    
    /* Dynamic allocation with error checking */
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    int *C = (int*)malloc(total_size * sizeof(int));
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size * sizeof(float));
    double *D = (double*)malloc(total_size * sizeof(double));
    int *indices = (int*)malloc(total_size * sizeof(int));
    int *mask = (int*)malloc(n * sizeof(int));
    
    if (!A || !B || !C || !X || !Y || !D || !indices || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern-based data */
    init_arrays(A, B, C, X, Y, D, indices, mask, total_size);
    
    printf("Starting OpenMP target offloading tests...\n");
    
    /* Execute test functions with different OpenMP constructs */
    test_simt_nested(A, B, C, n, m);
    
    test_simt_mapped(X, Y, indices, total_size, 16);
    
    test_simt_conditional(D, mask, n, m);
    
    /* Test with smaller size for mixed constructs */
    int test_size = (total_size > 10000) ? 10000 : total_size;
    test_mixed_constructs(A, test_size, 5);
    
    /* Verify results with checksum */
    long long checksum = compute_checksum(A, X, D, test_size);
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional verification - check for any obvious errors */
    int error_count = 0;
    #pragma omp parallel for reduction(+:error_count)
    for (int i = 0; i < test_size; i++) {
        if (isnan(X[i]) || isinf(X[i]) || isnan(D[i]) || isinf(D[i])) {
            error_count++;
        }
    }
    
    printf("Error count (NaN/Inf): %d\n", error_count);
    
    /* Cleanup */
    free(A); free(B); free(C);
    free(X); free(Y); free(D);
    free(indices); free(mask);
    
    return 0;
}
