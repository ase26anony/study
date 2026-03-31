#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 5381

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution path that depends on thread/loop indices */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] / 2 + C[idx];
            } else {
                A[idx] = B[idx] + C[idx] * 3;
            }
            
            /* Additional control flow with thread index check */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD clause */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simd safelen(16)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern */
        int idx = indices[i % size];
        if (idx >= 0 && idx < size) {
            /* Complex expression with conditional */
            float temp = Y[idx] * 2.5f;
            if (temp > 100.0f) {
                X[i] = sqrtf(temp) + sinf((float)i * 0.1f);
            } else {
                X[i] = temp * temp + cosf((float)idx * 0.05f);
            }
            
            /* SIMD-width dependent operation */
            for (int k = 0; k < 4; k++) {
                if ((i + k) % 8 == 0) {
                    X[i] += k * 0.5f;
                }
            }
        }
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp teams distribute
        for (int i = 0; i < rows; i++) {
            #pragma omp parallel for simd
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Thread-dependent conditional */
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((tid % 2) == (team_id % 3)) {
                    D[idx] = D[idx] * 1.5 + (double)(tid * team_id) * 0.01;
                } else {
                    D[idx] = D[idx] * 0.5 - (double)(tid + team_id) * 0.02;
                }
                
                /* Mask-based conditional */
                if (mask[idx] > 0) {
                    D[idx] = exp(D[idx] * 0.1);
                } else {
                    D[idx] = log(fabs(D[idx]) + 1.0);
                }
            }
        }
    }
}

/* Test 4: Mixed SIMD and non-SIMD constructs */
void test_mixed_simd(int *out, const int *in1, const int *in2, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: n, in1[0:n], in2[0:n]) map(tofrom: out[0:n])
    for (int i = 0; i < n; i++) {
        /* Complex conditional chain */
        int val1 = in1[i];
        int val2 = in2[i];
        
        if (val1 > val2) {
            out[i] = val1 - val2;
        } else if (val1 < val2) {
            out[i] = val2 - val1;
        } else {
            out[i] = val1 * val2;
        }
        
        /* SIMD-lane dependent operation */
        int lane = i % 32;
        if (lane < 16) {
            out[i] += lane;
        } else {
            out[i] -= (32 - lane);
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
    int base_size = 1000;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    /* Dynamic allocation with varying sizes */
    int n = base_size;
    int m = base_size / 2;
    int total_int = n * m;
    int total_float = base_size * 2;
    int total_double = (base_size / 4) * (base_size / 8);
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *mask = (int *)malloc(total_double * sizeof(int));
    
    float *X = (float *)malloc(total_float * sizeof(float));
    float *Y = (float *)malloc(total_float * sizeof(float));
    int *indices = (int *)malloc(total_float * sizeof(int));
    
    double *D = (double *)malloc(total_double * sizeof(double));
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 71) * 1.5f;
        indices[i] = (i * 7) % total_float;
    }
    
    for (int i = 0; i < total_double; i++) {
        D[i] = (double)(i % 59) * 0.7;
        mask[i] = (i % 5 == 0) ? 1 : -1;
    }
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        int offset = iter % 10;
        
        /* Test 1: Nested loops with SIMD transformation */
        test_simt_nested(A, B, C, n + offset, m - offset);
        
        /* Test 2: Pointer-based accesses with SIMD clause */
        test_simt_mapped(X, Y, indices, total_float, 1 + (iter % 4));
        
        /* Test 3: Nested parallel regions */
        test_simt_conditional(D, mask, base_size / 4 + offset, base_size / 8 - offset);
        
        /* Test 4: Mixed constructs */
        test_mixed_simd(A, B, C, total_int / 2);
        
        /* Modify some inputs for next iteration */
        if (iter % 3 == 0) {
            for (int i = 0; i < total_int; i += 100) {
                B[i] = (B[i] + iter) % 256;
                C[i] = (C[i] * 2) % 256;
            }
        }
    }
    
    /* Compute and print checksums */
    unsigned long checksum_A = compute_checksum(A, total_int * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, total_float * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, total_double * sizeof(double));
    
    printf("Checksums:\n");
    printf("  Array A: %lu\n", checksum_A);
    printf("  Array X: %lu\n", checksum_X);
    printf("  Array D: %lu\n", checksum_D);
    
    /* Cleanup */
    free(A); free(B); free(C); free(mask);
    free(X); free(Y); free(indices);
    free(D);
    
    return 0;
}
