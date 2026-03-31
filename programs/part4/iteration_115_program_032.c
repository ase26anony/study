#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 12345

/* Function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution path - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] / 2 + C[idx];
            } else {
                A[idx] = B[idx] + C[idx] * 3;
            }
            
            /* Additional control flow with thread index check */
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Function 2: Complex pointer-based accesses with SIMD safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size*stride]) \
        map(to: Y[0:size*stride], indices[0:size]) \
        simd safelen(16)
    for (int i = 0; i < size; i++) {
        int base_idx = i * stride;
        int src_idx = indices[i] * stride;
        
        /* Indirect memory access pattern - may influence SIMT decisions */
        for (int k = 0; k < stride; k++) {
            X[base_idx + k] = Y[src_idx + k] * 2.0f;
        }
        
        /* Conditional based on computed value */
        if (X[base_idx] > 100.0f) {
            X[base_idx] = sqrtf(X[base_idx]);
        }
        
        /* SIMD-unfriendly pattern to force transformation */
        if (i % 8 == 0) {
            X[base_idx] += omp_get_team_num() * 0.1f;
        }
    }
}

/* Function 3: Separate parallel and SIMD regions with thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int len, int offset) {
    #pragma omp target map(to: len, offset) map(tofrom: D[0:len]) map(to: mask[0:len])
    {
        #pragma omp parallel for simd
        for (int i = 0; i < len; i++) {
            /* Thread-dependent condition - may trigger the label/cond structure */
            int thread_id = omp_get_thread_num();
            
            if (thread_id % 2 == 0) {
                D[i] = D[i] * 2.0 + mask[i];
            } else {
                D[i] = D[i] / 2.0 - mask[i];
            }
            
            /* Nested condition with loop variable */
            if (i % 16 < 8) {
                D[i] += sin((double)i * 0.1);
            } else {
                D[i] += cos((double)i * 0.1);
            }
        }
        
        /* Additional parallel region with distribute */
        #pragma omp teams distribute parallel for simd
        for (int i = offset; i < len - offset; i++) {
            D[i] = D[i] * 0.5 + D[i-1] * 0.3 + D[i+1] * 0.2;
        }
    }
}

/* Function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *arr1, int *arr2, int dim1, int dim2) {
    /* First target region: teams distribute */
    #pragma omp target teams distribute \
        map(to: dim1, dim2) map(tofrom: arr1[0:dim1*dim2]) \
        map(to: arr2[0:dim1*dim2])
    for (int i = 0; i < dim1; i++) {
        #pragma omp parallel for simd
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            arr1[idx] = arr2[idx] + i * j;
            
            /* Complex condition to force control flow */
            if ((arr1[idx] & 0xF) == 0) {
                arr1[idx] = arr1[idx] >> 1;
            } else if ((arr1[idx] & 0xF) == 1) {
                arr1[idx] = arr1[idx] << 1;
            }
        }
    }
    
    /* Second target region: direct parallel for simd */
    #pragma omp target parallel for simd \
        map(to: dim1, dim2) map(tofrom: arr1[0:dim1*dim2])
    for (int i = 0; i < dim1 * dim2; i++) {
        arr1[i] = arr1[i] % 256;
    }
}

/* Compute checksum to verify execution */
unsigned long long compute_checksum(void *data, size_t size) {
    unsigned long long checksum = CHECKSUM_SEED;
    unsigned char *bytes = (unsigned char *)data;
    
    for (size_t i = 0; i < size; i++) {
        checksum = checksum * 31 + bytes[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int n = 1000, m = 200;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    printf("Running SIMT transformation test with n=%d, m=%d\n", n, m);
    
    /* Allocate and initialize arrays with pattern-based data */
    int total_int = n * m;
    int total_float = n * 16;  /* stride = 16 for test_simt_mapped */
    
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *mask = (int *)malloc(total_int * sizeof(int));
    
    float *X = (float *)malloc(total_float * sizeof(float));
    float *Y = (float *)malloc(total_float * sizeof(float));
    int *indices = (int *)malloc(n * sizeof(int));
    
    double *D = (double *)malloc(total_int * sizeof(double));
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        mask[i] = (i % 2 == 0) ? 1 : -1;
        D[i] = (double)(i % 100) / 10.0;
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 50) * 1.5f;
    }
    
    for (int i = 0; i < n; i++) {
        indices[i] = (i * 7) % n;
    }
    
    /* Execute test functions with different OpenMP constructs */
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, n, 16);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, total_int, 10);
    
    printf("Running test_mixed_constructs...\n");
    test_mixed_constructs(A, B, n, m);
    
    /* Compute and print checksums to verify execution */
    unsigned long long checksum_A = compute_checksum(A, total_int * sizeof(int));
    unsigned long long checksum_X = compute_checksum(X, total_float * sizeof(float));
    unsigned long long checksum_D = compute_checksum(D, total_int * sizeof(double));
    
    printf("Checksum A: %llu\n", checksum_A);
    printf("Checksum X: %llu\n", checksum_X);
    printf("Checksum D: %llu\n", checksum_D);
    
    /* Cleanup */
    free(A); free(B); free(C); free(mask);
    free(X); free(Y); free(indices);
    free(D);
    
    printf("Test completed successfully.\n");
    return 0;
}
