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
        default(none) shared(A, B, C)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution path that depends on thread/loop indices */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] - C[idx];
            } else {
                /* Complex expression to prevent optimization */
                A[idx] = (B[idx] << 1) | (C[idx] & 0xFF);
            }
            
            /* Additional conditional with thread-specific behavior */
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += idx;
            }
        }
    }
}

/* Test 2: Mapped pointers with indirect access and safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, indices[0:size], Y[0:size*stride]) \
        map(tofrom: X[0:size]) \
        safelen(16) aligned(X:64, Y:64) \
        default(none) shared(X, Y, indices)
    for (int i = 0; i < size; i++) {
        int base_idx = indices[i] * stride;
        
        /* Indirect access pattern - important for SIMT memory coalescing */
        float sum = 0.0f;
        for (int k = 0; k < 4; k++) {
            sum += Y[base_idx + k];
        }
        
        /* Conditional based on SIMD lane-like computation */
        if (i % 8 < 4) {
            X[i] = sum * 2.0f;
        } else {
            X[i] = sum / 2.0f;
        }
        
        /* Use of math function that might need special handling */
        X[i] += sinf((float)i * 0.1f) * 0.5f;
    }
}

/* Test 3: Separate parallel and simd constructs with thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int len, int threshold) {
    #pragma omp target map(to: len, threshold, mask[0:len]) \
        map(tofrom: D[0:len]) default(none) shared(D, mask)
    {
        #pragma omp parallel for simd \
            if(len > 1000) \
            default(none) shared(D, mask, threshold)
        for (int i = 0; i < len; i++) {
            /* Complex condition depending on thread number */
            int thread_mod = omp_get_thread_num() % 8;
            
            if (thread_mod == 0) {
                D[i] = D[i] * 2.0 + (double)mask[i];
            } else if (thread_mod < 4) {
                D[i] = sqrt(D[i] + (double)(i % threshold));
            } else {
                D[i] = D[i] / (1.0 + (double)thread_mod);
            }
            
            /* Nested condition with loop index dependency */
            if ((i & 0xF) == 0 && thread_mod % 2 == 1) {
                D[i] = -D[i];
            }
        }
    }
}

/* Test 4: Mixed directives to explore different lowering paths */
void test_mixed_constructs(int *out, const int *in1, const int *in2, 
                          int rows, int cols, int depth) {
    /* First target region with teams distribute */
    #pragma omp target teams distribute \
        map(to: rows, cols, depth, in1[0:rows*cols], in2[0:rows*depth]) \
        map(tofrom: out[0:rows*cols]) \
        default(none) shared(out, in1, in2)
    for (int i = 0; i < rows; i++) {
        #pragma omp parallel for simd collapse(2) \
            default(none) shared(out, in1, in2, i)
        for (int j = 0; j < cols; j++) {
            for (int k = 0; k < depth; k++) {
                int idx = i * cols + j;
                int idx2 = i * depth + k;
                
                /* Complex computation with multiple conditions */
                int val = in1[idx] + in2[idx2];
                if (j % 3 == 0) {
                    val *= 2;
                }
                if (k % 2 == 0) {
                    val += (i + j + k);
                }
                
                /* Write with conditional */
                out[idx] = (val > 0) ? val : -val;
            }
        }
    }
}

/* Helper function to compute checksum */
unsigned long compute_checksum(void *data, size_t size_bytes) {
    unsigned long checksum = 0;
    unsigned char *bytes = (unsigned char *)data;
    
    for (size_t i = 0; i < size_bytes; i++) {
        checksum = (checksum * 31 + bytes[i]) % CHECKSUM_MOD;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int base_size = 512;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    /* Dynamically allocate arrays with varying sizes */
    int n = base_size;
    int m = base_size / 2;
    int size1 = n * m;
    int size2 = base_size * 64;
    int size3 = base_size * 4;
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(size1 * sizeof(int));
    int *B = (int *)malloc(size1 * sizeof(int));
    int *C = (int *)malloc(size1 * sizeof(int));
    
    float *X = (float *)malloc(size2 * sizeof(float));
    float *Y = (float *)malloc(size2 * 2 * sizeof(float));
    int *indices = (int *)malloc(size2 * sizeof(int));
    
    double *D = (double *)malloc(size3 * sizeof(double));
    int *mask = (int *)malloc(size3 * sizeof(int));
    
    int *out = (int *)malloc(size1 * sizeof(int));
    int *in1 = (int *)malloc(size1 * sizeof(int));
    int *in2 = (int *)malloc(n * (m/2) * sizeof(int));
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < size1; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        in1[i] = i % 73;
        out[i] = 0;
    }
    
    for (int i = 0; i < size2; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 127) * 0.1f;
        indices[i] = i % (size2 / 2);
    }
    
    for (int i = 0; i < size3; i++) {
        D[i] = (double)(i % 89) * 0.5;
        mask[i] = (i % 13) > 6 ? 1 : 0;
    }
    
    for (int i = 0; i < n * (m/2); i++) {
        in2[i] = (i * 7) % 67;
    }
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        int offset = iter % 16;
        
        /* Modify parameters slightly each iteration */
        test_simt_nested(A + offset, B + offset, C + offset, 
                        n - offset, m - offset/2);
        
        test_simt_mapped(X + offset, Y + offset * 2, indices + offset,
                        size2 - offset * 4, 8 + (iter % 8));
        
        test_simt_conditional(D + offset, mask + offset,
                             size3 - offset * 8, 50 + (iter % 30));
        
        if (iter % 4 == 0) {
            test_mixed_constructs(out + offset, in1 + offset, in2,
                                 n - offset, m - offset/2, m/2);
        }
    }
    
    /* Compute and print checksums to ensure all code executed */
    unsigned long checksum_A = compute_checksum(A, size1 * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, size2 * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, size3 * sizeof(double));
    unsigned long checksum_out = compute_checksum(out, size1 * sizeof(int));
    
    printf("Checksums: A=%lu, X=%lu, D=%lu, out=%lu\n",
           checksum_A, checksum_X, checksum_D, checksum_out);
    
    /* Cleanup */
    free(A); free(B); free(C);
    free(X); free(Y); free(indices);
    free(D); free(mask);
    free(out); free(in1); free(in2);
    
    return 0;
}
