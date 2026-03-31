#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 12345

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional execution based on thread index and loop variables */
            if ((omp_get_thread_num() % 3) == (i % 3)) {
                A[idx] = B[idx] + C[idx] * (i + j);
            } else {
                A[idx] = B[idx] - C[idx] * (i - j);
            }
            
            /* Additional conditional to create more complex control flow */
            if (j % 7 == 0) {
                A[idx] += (i % 5) * 2;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD clause */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32)
    for (int i = 0; i < size; i += stride) {
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            /* Indirect access pattern that requires memory coalescing */
            X[i] = Y[idx] * 2.0f + X[i] * 0.5f;
            
            /* Conditional store based on computed value */
            if (X[i] > 100.0f) {
                X[i] = sqrtf(X[i]);
            } else if (X[i] < -50.0f) {
                X[i] = -sqrtf(-X[i]);
            }
        }
        
        /* SIMD lane-dependent computation */
        int lane = omp_get_thread_num() % 32;
        X[i] += lane * 0.01f;
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target teams distribute map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    for (int i = 0; i < rows; i++) {
        #pragma omp parallel for simd
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex conditional based on thread ID and mask */
            int tid = omp_get_thread_num();
            if ((tid % 4) == (mask[idx] % 4)) {
                D[idx] = sin(D[idx]) * cos((double)j);
            } else {
                D[idx] = cos(D[idx]) * sin((double)i);
            }
            
            /* Additional SIMD-specific branching */
            if (j % 16 == 0) {
                D[idx] += 1.0 / (1.0 + D[idx] * D[idx]);
            }
        }
    }
}

/* Test 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int n, int m) {
    /* First a simple target region */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            data[i * m + j] += i * j;
        }
    }
    
    /* Then a SIMD-specific region */
    #pragma omp target teams distribute simd \
        map(tofrom: data[0:n*m])
    for (int i = 0; i < n * m; i++) {
        if (data[i] % 2 == 0) {
            data[i] = data[i] * 3 + 1;
        } else {
            data[i] = data[i] / 2;
        }
    }
}

/* Compute checksum to verify execution */
unsigned long compute_checksum(void *data, size_t size) {
    unsigned long checksum = CHECKSUM_SEED;
    unsigned char *bytes = (unsigned char *)data;
    
    for (size_t i = 0; i < size; i++) {
        checksum = (checksum * 31) + bytes[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int n = 512;
    int m = 256;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    if (n <= 0) n = 512;
    if (m <= 0) m = 256;
    
    printf("Testing OpenMP SIMT transformation with n=%d, m=%d\n", n, m);
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(n * m * sizeof(int));
    int *B = (int *)malloc(n * m * sizeof(int));
    int *C = (int *)malloc(n * m * sizeof(int));
    float *X = (float *)malloc(n * m * sizeof(float));
    float *Y = (float *)malloc(n * m * sizeof(float));
    int *indices = (int *)malloc(n * m * sizeof(int));
    double *D = (double *)malloc(n * m * sizeof(double));
    int *mask = (int *)malloc(n * m * sizeof(int));
    
    if (!A || !B || !C || !X || !Y || !indices || !D || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < n * m; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        X[i] = (float)(i % 100) * 0.1f;
        Y[i] = (float)((i + 7) % 100) * 0.2f;
        indices[i] = (i * 2) % (n * m);
        D[i] = (double)(i % 200) * 0.05;
        mask[i] = i % 11;
    }
    
    /* Execute test functions with different OpenMP constructs */
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, n * m, 1);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, n, m);
    
    printf("Running test_mixed_constructs...\n");
    test_mixed_constructs(A, n, m);
    
    /* Compute and print checksums to verify execution */
    unsigned long checksum_A = compute_checksum(A, n * m * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, n * m * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, n * m * sizeof(double));
    
    printf("Checksums:\n");
    printf("  Array A: %lu\n", checksum_A);
    printf("  Array X: %lu\n", checksum_X);
    printf("  Array D: %lu\n", checksum_D);
    
    /* Cleanup */
    free(A);
    free(B);
    free(C);
    free(X);
    free(Y);
    free(indices);
    free(D);
    free(mask);
    
    printf("Test completed successfully\n");
    return 0;
}
