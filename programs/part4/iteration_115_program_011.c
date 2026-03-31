#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 5381

/* Function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution inside SIMD loop - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] - C[idx];
            } else {
                A[idx] = B[idx] + C[idx] * 3;
            }
            
            /* Additional control flow with thread-dependent condition */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Function 2: Complex pointer-based accesses with SIMD clause */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size*2], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Indirect access pattern - encourages SIMT for memory coalescing */
        int idx = indices[i % size];
        
        /* Complex conditional with floating point operations */
        if (Y[idx] > 0.5f) {
            X[i] = Y[idx] * 2.0f + sinf((float)i * 0.1f);
        } else {
            X[i] = Y[idx * 2] * 0.5f - cosf((float)i * 0.05f);
        }
        
        /* Nested condition based on loop index */
        if (i % 8 == 0) {
            X[i] = fabsf(X[i]);
        } else if (i % 8 == 4) {
            X[i] = -X[i];
        }
    }
}

/* Function 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target teams distribute map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    for (int i = 0; i < rows; i++) {
        #pragma omp parallel for simd
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Thread-dependent conditional - may trigger IFN_GOMP_USE_SIMT */
            int tid = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            if ((tid ^ team_id) % 2 == 0) {
                D[idx] = (double)mask[idx] * 1.5;
            } else {
                D[idx] = (double)mask[idx] * 0.75;
            }
            
            /* Additional branching based on computed value */
            if (D[idx] > 100.0) {
                D[idx] = sqrt(D[idx]);
            }
        }
    }
}

/* Function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *arr1, int *arr2, int n) {
    /* First: target teams distribute simd */
    #pragma omp target teams distribute simd \
        map(tofrom: arr1[0:n]) map(to: arr2[0:n])
    for (int i = 0; i < n; i++) {
        if (i % 16 < 8) {
            arr1[i] = arr2[i] << 2;
        } else {
            arr1[i] = arr2[i] >> 1;
        }
    }
    
    /* Second: target parallel for simd */
    #pragma omp target parallel for simd \
        map(tofrom: arr1[0:n]) map(to: arr2[0:n])
    for (int i = 0; i < n; i++) {
        arr1[i] += arr2[n - i - 1];
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
    int n = 1000;
    int m = 200;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    if (n <= 0) n = 1000;
    if (m <= 0) m = 200;
    
    printf("Testing OpenMP SIMT transformation with n=%d, m=%d\n", n, m);
    
    /* Allocate and initialize arrays with non-constant patterns */
    int *A = (int *)malloc(n * m * sizeof(int));
    int *B = (int *)malloc(n * m * sizeof(int));
    int *C = (int *)malloc(n * m * sizeof(int));
    float *X = (float *)malloc(n * sizeof(float));
    float *Y = (float *)malloc(n * 2 * sizeof(float));
    int *indices = (int *)malloc(n * sizeof(int));
    double *D = (double *)malloc(n * m * sizeof(double));
    int *mask = (int *)malloc(n * m * sizeof(int));
    
    if (!A || !B || !C || !X || !Y || !indices || !D || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data (non-constant) */
    for (int i = 0; i < n * m; i++) {
        A[i] = 0;
        B[i] = (i * 17) % 97;
        C[i] = (i * 23) % 101;
        if (i < n * m) {
            D[i] = (double)(i % 50);
            mask[i] = (i % 7 == 0) ? 1 : 0;
        }
    }
    
    for (int i = 0; i < n; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 100) / 100.0f;
        Y[i + n] = (float)((i * 3) % 100) / 100.0f;
        indices[i] = (i * 7) % n;
    }
    
    printf("Initialization complete. Starting OpenMP target regions...\n");
    
    /* Execute test functions with different constructs */
    test_simt_nested(A, B, C, n, m);
    printf("test_simt_nested completed\n");
    
    test_simt_mapped(X, Y, indices, n, 2);
    printf("test_simt_mapped completed\n");
    
    test_simt_conditional(D, mask, n, m);
    printf("test_simt_conditional completed\n");
    
    test_mixed_constructs(A, B, n * m);
    printf("test_mixed_constructs completed\n");
    
    /* Compute and print checksums to verify execution */
    unsigned long checksum_A = compute_checksum(A, n * m * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, n * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, n * m * sizeof(double));
    
    printf("\nChecksums (verify execution):\n");
    printf("Array A: %lu\n", checksum_A);
    printf("Array X: %lu\n", checksum_X);
    printf("Array D: %lu\n", checksum_D);
    
    /* Cleanup */
    free(A);
    free(B);
    free(C);
    free(X);
    free(Y);
    free(indices);
    free(D);
    free(mask);
    
    printf("\nTest completed successfully.\n");
    
    return 0;
}
