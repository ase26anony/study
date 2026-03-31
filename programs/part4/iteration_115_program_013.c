#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with collapse and SIMD clause */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() + i + j) % 4 == 0) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((omp_get_thread_num() + i) % 3 == 0) {
                A[idx] = B[idx] - C[idx] + iter;
            } else {
                A[idx] = B[idx] + C[idx] - iter;
            }
            
            /* Additional control flow with nested if */
            if (j % 8 == 0) {
                A[idx] += (i % 16);
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        int idx = indices[i];
        
        /* Indirect memory access pattern */
        if (idx >= 0 && idx < size) {
            float temp = Y[idx];
            
            /* Conditional transformation */
            if (omp_get_thread_num() % 2 == 0) {
                X[i] = temp * temp + sinf((float)i * 0.1f);
            } else {
                X[i] = sqrtf(fabsf(temp)) + cosf((float)i * 0.05f);
            }
            
            /* Additional SIMD-friendly but conditional operation */
            if (i % 4 == 0) {
                X[i] += 1.0f / (1.0f + (float)(idx % 8));
            }
        }
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int offset) {
    #pragma omp target map(to: rows, cols, offset) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp teams distribute
        for (int i = 0; i < rows; i++) {
            #pragma omp parallel for simd
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Complex condition depending on multiple factors */
                int thread_id = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((thread_id ^ team_id ^ (i * j)) % 5 == 0) {
                    D[idx] = (double)mask[idx] * 2.5 + offset;
                } else if ((thread_id + j) % 3 == 0) {
                    D[idx] = (double)mask[idx] / 3.0 - offset;
                } else {
                    D[idx] = (double)(mask[idx] % 100) + sin((double)(i + j) * 0.01);
                }
                
                /* Nested condition inside SIMD loop */
                if (j > cols / 2) {
                    D[idx] *= 1.1;
                }
            }
        }
    }
}

/* Test 4: Mixed directives to explore different lowering paths */
void test_mixed_simd(int *data, int dim1, int dim2, int dim3) {
    /* First target region with distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: dim1, dim2) map(tofrom: data[0:dim1*dim2*dim3])
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                int idx = (i * dim2 + j) * dim3 + k;
                
                /* Conditional that depends on all loop indices */
                if (((i * 17 + j * 13 + k * 7) % 11) < (omp_get_thread_num() % 7)) {
                    data[idx] = (data[idx] * 3) % 65536;
                } else {
                    data[idx] = (data[idx] + 256) % 65536;
                }
            }
        }
    }
    
    /* Second target region with different structure */
    #pragma omp target teams distribute simd \
        map(to: dim1, dim2, dim3) map(tofrom: data[0:dim1*dim2*dim3])
    for (int i = 0; i < dim1 * dim2 * dim3; i++) {
        if (data[i] % 2 == (omp_get_thread_num() % 2)) {
            data[i] = data[i] ^ 0x00FF00FF;
        }
    }
}

/* Helper function to compute checksum */
unsigned long long compute_checksum(void *array, size_t size_bytes) {
    unsigned long long checksum = 0;
    unsigned char *bytes = (unsigned char *)array;
    
    for (size_t i = 0; i < size_bytes; i++) {
        checksum = (checksum * 31 + bytes[i]) % CHECKSUM_MOD;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int base_size = 1000;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running SIMT tests with size=%d, iterations=%d\n", base_size, iterations);
    
    /* Calculate various sizes for different tests */
    int n = base_size;
    int m = base_size / 2;
    int total_int = n * m;
    int total_float = base_size * 2;
    int rows = base_size / 4;
    int cols = base_size / 8;
    int total_double = rows * cols;
    int mixed_dims = base_size / 16;
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    
    float *X = (float *)malloc(total_float * sizeof(float));
    float *Y = (float *)malloc(total_float * sizeof(float));
    int *indices = (int *)malloc(total_float * sizeof(int));
    
    double *D = (double *)malloc(total_double * sizeof(double));
    int *mask = (int *)malloc(total_double * sizeof(int));
    
    int *mixed_data = (int *)malloc(mixed_dims * mixed_dims * mixed_dims * sizeof(int));
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        C[i] = (i * 7) % 113;
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 59) * 0.1f;
        indices[i] = (i * 11) % total_float;
    }
    
    for (int i = 0; i < total_double; i++) {
        D[i] = 0.0;
        mask[i] = (i * 13) % 71;
    }
    
    for (int i = 0; i < mixed_dims * mixed_dims * mixed_dims; i++) {
        mixed_data[i] = (i * 19) % 255;
    }
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        test_simt_nested(A, B, C, n, m, iter);
        
        if (iter % 3 == 0) {
            test_simt_mapped(X, Y, indices, total_float, 1 + (iter % 4));
        }
        
        if (iter % 5 == 0) {
            test_simt_conditional(D, mask, rows, cols, iter);
        }
        
        if (iter % 7 == 0) {
            test_mixed_simd(mixed_data, mixed_dims, mixed_dims, mixed_dims);
        }
    }
    
    /* Compute and print checksums */
    unsigned long long checksum_A = compute_checksum(A, total_int * sizeof(int));
    unsigned long long checksum_X = compute_checksum(X, total_float * sizeof(float));
    unsigned long long checksum_D = compute_checksum(D, total_double * sizeof(double));
    unsigned long long checksum_mixed = compute_checksum(mixed_data, 
        mixed_dims * mixed_dims * mixed_dims * sizeof(int));
    
    printf("Checksums:\n");
    printf("  Array A: %llu\n", checksum_A);
    printf("  Array X: %llu\n", checksum_X);
    printf("  Array D: %llu\n", checksum_D);
    printf("  Mixed data: %llu\n", checksum_mixed);
    
    /* Cleanup */
    free(A); free(B); free(C);
    free(X); free(Y); free(indices);
    free(D); free(mask);
    free(mixed_data);
    
    return 0;
}
