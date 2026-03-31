#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 5381

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        default(none) shared(A, B, C)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional that depends on thread/loop indices */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2;
            } else if ((i * j) % 5 == 1) {
                A[idx] = B[idx] + C[idx];
            } else {
                A[idx] = B[idx] - C[idx];
            }
            
            /* Additional control flow to complicate SIMD transformation */
            if (j % 7 == 0) {
                A[idx] += (i % 2 == 0) ? 1 : -1;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, indices[0:size], Y[0:size*stride]) \
        map(tofrom: X[0:size]) \
        safelen(16) \
        default(none) shared(X, Y, indices)
    for (int i = 0; i < size; i++) {
        int base_idx = indices[i] * stride;
        
        /* Indirect access pattern that requires memory coalescing */
        float sum = 0.0f;
        for (int k = 0; k < 4; k++) {
            sum += Y[base_idx + k];
        }
        
        /* Conditional store based on computed value */
        if (sum > 0.0f) {
            X[i] = sum * (i % 8 + 1);
        } else {
            X[i] = -sum / (i % 8 + 1);
        }
        
        /* SIMD-unfriendly pattern */
        X[i] += (indices[i] % 3 == 0) ? 1.5f : 0.5f;
    }
}

/* Test 3: Nested parallel region with thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int len, int offset) {
    #pragma omp target map(to: len, offset, mask[0:len]) \
        map(tofrom: D[0:len]) default(none) shared(D, mask)
    {
        #pragma omp parallel for simd
        for (int i = 0; i < len; i++) {
            int thread_id = omp_get_thread_num();
            
            /* Thread-dependent condition - forces divergence */
            if (thread_id % 2 == 0) {
                D[i] = sin(D[i] * mask[i]) + offset;
            } else {
                D[i] = cos(D[i] * mask[i]) - offset;
            }
            
            /* Additional condition based on loop index */
            if (i % (thread_id + 1) == 0) {
                D[i] *= 1.1;
            }
        }
    }
}

/* Test 4: Multiple SIMD clauses with private variables */
void test_simt_private(int *out, const int *in1, const int *in2, int dim1, int dim2) {
    #pragma omp target teams distribute parallel for simd \
        map(to: dim1, dim2, in1[0:dim1*dim2], in2[0:dim1*dim2]) \
        map(tofrom: out[0:dim1*dim2]) \
        private(i, j, temp) \
        collapse(2) \
        default(none) shared(out, in1, in2)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            int temp = in1[idx] + in2[idx];
            
            /* Complex conditional chain */
            if (temp > 100) {
                out[idx] = temp % 97;
            } else if (temp < -100) {
                out[idx] = (-temp) % 97;
            } else {
                out[idx] = (temp * temp) % 97;
            }
            
            /* SIMD lane-dependent operation */
            out[idx] += (j % 16) * 2;
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
    int n = 512;
    int m = 256;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d\n", n, m);
    
    /* Allocate and initialize arrays */
    int total_int = n * m;
    int total_float = n * 16;  /* For stride access */
    
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *indices = (int *)malloc(n * sizeof(int));
    float *X = (float *)malloc(n * sizeof(float));
    float *Y = (float *)malloc(total_float * sizeof(float));
    double *D = (double *)malloc(n * sizeof(double));
    int *mask = (int *)malloc(n * sizeof(int));
    int *out = (int *)malloc(total_int * sizeof(int));
    int *in1 = (int *)malloc(total_int * sizeof(int));
    int *in2 = (int *)malloc(total_int * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask || !out || !in1 || !in2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 97;
        in1[i] = i % 73;
        in2[i] = (i * 2) % 73;
        out[i] = 0;
    }
    
    for (int i = 0; i < n; i++) {
        indices[i] = i % 16;
        X[i] = (float)(i % 29) * 0.1f;
        D[i] = (double)(i % 31) * 0.01;
        mask[i] = (i % 3 == 0) ? 1 : -1;
    }
    
    for (int i = 0; i < total_float; i++) {
        Y[i] = (float)(i % 43) * 0.05f;
    }
    
    printf("Initial checksums:\n");
    printf("  A: %lu\n", compute_checksum(A, total_int * sizeof(int)));
    printf("  B: %lu\n", compute_checksum(B, total_int * sizeof(int)));
    
    /* Execute test functions with different OpenMP constructs */
    printf("\nExecuting test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    printf("Executing test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, n, 16);
    
    printf("Executing test_simt_conditional...\n");
    test_simt_conditional(D, mask, n, 5);
    
    printf("Executing test_simt_private...\n");
    test_simt_private(out, in1, in2, n, m);
    
    /* Compute final checksums */
    printf("\nFinal checksums:\n");
    printf("  A: %lu\n", compute_checksum(A, total_int * sizeof(int)));
    printf("  X: %lu\n", compute_checksum(X, n * sizeof(float)));
    printf("  D: %lu\n", compute_checksum(D, n * sizeof(double)));
    printf("  out: %lu\n", compute_checksum(out, total_int * sizeof(int)));
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices);
    free(X); free(Y); free(D); free(mask);
    free(out); free(in1); free(in2);
    
    printf("\nTest completed successfully.\n");
    return 0;
}
