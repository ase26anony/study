#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional that depends on thread/iteration pattern */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i * j) % 5 == 1) {
                A[idx] = B[idx] - C[idx];
            } else {
                A[idx] = B[idx] + C[idx] + 1;
            }
            
            /* Additional control flow to complicate SIMD transformation */
            if (j % 7 == 0) {
                A[idx] += (i % 2) * 10;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, Y[0:size*2], indices[0:size]) \
        map(tofrom: X[0:size*2]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i++) {
        int idx1 = indices[i];
        int idx2 = indices[(i + stride) % size];
        
        /* Indirect memory access pattern */
        X[i] = Y[idx1] * 2.0f - Y[idx2];
        
        /* Conditional based on computed value */
        if (X[i] > 100.0f) {
            X[i] = sqrtf(fabsf(X[i]));
        } else if (X[i] < -50.0f) {
            X[i] = X[i] * 0.5f + 25.0f;
        }
        
        /* Additional SIMD-unfriendly operation */
        X[size + i] = (i % 8 == 0) ? X[i] * 3.0f : X[i] / 2.0f;
    }
}

/* Test 3: Nested parallel region with thread-dependent condition */
void test_simt_conditional(double *D, int *flags, int n, int offset) {
    #pragma omp target map(to: n, offset, flags[0:n]) map(tofrom: D[0:n]) \
        defaultmap(tofrom:scalar)
    {
        #pragma omp parallel for simd
        for (int i = 0; i < n; i++) {
            int tid = omp_get_thread_num();
            
            /* Thread-dependent conditional - forces divergence */
            if (tid % 2 == 0) {
                D[i] = sin((double)(i + offset) * 0.1) * flags[i];
            } else {
                D[i] = cos((double)(i + offset) * 0.1) * (flags[i] + 1);
            }
            
            /* Nested condition based on computed value */
            if (D[i] > 0.5) {
                D[i] = D[i] * 0.8;
            } else if (D[i] < -0.5) {
                D[i] = D[i] * 1.2;
            }
        }
    }
}

/* Test 4: Mixed directives with device_ptr */
void test_simt_deviceptr(int *src, int *dst, int n, int m) {
    int *dev_src, *dev_dst;
    
    #pragma omp target data map(to: n, m) \
        map(to: src[0:n*m]) map(from: dst[0:n*m]) \
        use_device_ptr(src, dst)
    {
        dev_src = src;
        dev_dst = dst;
        
        #pragma omp target teams distribute parallel for simd collapse(2) \
            is_device_ptr(dev_src, dev_dst)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                int val = dev_src[idx];
                
                /* Complex conditional chain */
                switch (val % 4) {
                    case 0:
                        dev_dst[idx] = val * 2;
                        break;
                    case 1:
                        dev_dst[idx] = val + (i % 10);
                        break;
                    case 2:
                        dev_dst[idx] = val - (j % 7);
                        break;
                    default:
                        dev_dst[idx] = val / 2;
                        if (dev_dst[idx] % 3 == 0) {
                            dev_dst[idx] += 100;
                        }
                }
            }
        }
    }
}

/* Helper function to compute checksum */
unsigned long long compute_checksum(void *data, size_t size, size_t elem_size) {
    unsigned long long checksum = 0;
    unsigned char *bytes = (unsigned char *)data;
    size_t total_bytes = size * elem_size;
    
    for (size_t i = 0; i < total_bytes; i++) {
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
    
    /* Dynamically allocate arrays with varying sizes */
    int n = base_size;
    int m = base_size / 2;
    int total_int = n * m;
    int total_float = n * 2;
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *indices = (int *)malloc(n * sizeof(int));
    int *flags = (int *)malloc(n * sizeof(int));
    int *src = (int *)malloc(total_int * sizeof(int));
    int *dst = (int *)malloc(total_int * sizeof(int));
    float *X = (float *)malloc(total_float * sizeof(float));
    float *Y = (float *)malloc(total_float * sizeof(float));
    double *D = (double *)malloc(n * sizeof(double));
    
    if (!A || !B || !C || !indices || !flags || !src || !dst || !X || !Y || !D) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        C[i] = (i * 7) % 113;
        src[i] = (i * 11) % 197;
        dst[i] = 0;
    }
    
    for (int i = 0; i < n; i++) {
        indices[i] = (i * 13) % n;
        flags[i] = (i % 3 == 0) ? 1 : -1;
        D[i] = (double)(i % 50) / 10.0;
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = 0.0f;
        Y[i] = (float)((i * 17) % 100) / 3.0f;
    }
    
    /* Execute test functions multiple times */
    for (int iter = 0; iter < iterations; iter++) {
        if (iter % 4 == 0) {
            test_simt_nested(A, B, C, n, m);
        }
        
        if (iter % 3 == 0) {
            test_simt_mapped(X, Y, indices, n, (iter % 7) + 1);
        }
        
        if (iter % 2 == 0) {
            test_simt_conditional(D, flags, n, iter);
        }
        
        if (iter % 5 == 0) {
            test_simt_deviceptr(src, dst, n, m);
        }
    }
    
    /* Compute and print checksums */
    unsigned long long checksum_A = compute_checksum(A, total_int, sizeof(int));
    unsigned long long checksum_X = compute_checksum(X, total_float, sizeof(float));
    unsigned long long checksum_D = compute_checksum(D, n, sizeof(double));
    unsigned long long checksum_dst = compute_checksum(dst, total_int, sizeof(int));
    
    printf("Checksums:\n");
    printf("  Array A: %llu\n", checksum_A);
    printf("  Array X: %llu\n", checksum_X);
    printf("  Array D: %llu\n", checksum_D);
    printf("  Array dst: %llu\n", checksum_dst);
    
    /* Verify some values */
    int verify_count = 0;
    for (int i = 0; i < 10 && i < total_int; i++) {
        if (A[i] != 0) verify_count++;
    }
    printf("Non-zero values in first 10 elements of A: %d\n", verify_count);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices); free(flags);
    free(src); free(dst); free(X); free(Y); free(D);
    
    return 0;
}
