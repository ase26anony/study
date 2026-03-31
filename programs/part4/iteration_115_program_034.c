#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <assert.h>

#define CHECKSUM_SEED 5381

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Complex conditional that depends on both loop indices */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * C[idx] + i - j;
            } else if ((i * j) % 5 == 2) {
                A[idx] = B[idx] / (C[idx] + 1) + (i ^ j);
            } else {
                A[idx] = B[idx] + C[idx] + (i & j);
            }
            
            /* Additional control flow with thread ID */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += tid;
            }
        }
    }
}

/* Test 2: Pointer-based indirect accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size*2], indices[0:size]) \
        safelen(16)
    for (int i = 0; i < size; i += stride) {
        int idx = indices[i];
        /* Complex pointer arithmetic and conditional */
        if (idx >= 0 && idx < size * 2) {
            X[i] = Y[idx] * 2.0f + Y[idx + 1] * 0.5f;
            
            /* Nested condition based on computed value */
            if (X[i] > 100.0f) {
                X[i] = 100.0f;
            } else if (X[i] < -100.0f) {
                X[i] = -100.0f;
            }
        } else {
            X[i] = 0.0f;
        }
        
        /* SIMD pragma inside the loop body for additional complexity */
        #pragma omp simd
        for (int k = 0; k < 4; k++) {
            X[i] += k * 0.1f;
        }
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int nrows, int ncols) {
    #pragma omp target map(tofrom: D[0:nrows*ncols]) map(to: mask[0:nrows]) \
        defaultmap(tofrom:scalar)
    {
        #pragma omp teams distribute
        for (int row = 0; row < nrows; row++) {
            #pragma omp parallel for simd
            for (int col = 0; col < ncols; col++) {
                int idx = row * ncols + col;
                
                /* Condition that depends on thread ID and loop variables */
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((tid + team_id) % 2 == 0) {
                    D[idx] = (mask[row] % 7) * tid + col * 0.5;
                } else {
                    D[idx] = (mask[row] % 11) * team_id - col * 0.25;
                }
                
                /* Additional branching based on computed value */
                if (D[idx] > 50.0) {
                    D[idx] = 50.0;
                } else if (D[idx] < -50.0) {
                    D[idx] = -50.0 + (tid % 3);
                }
            }
        }
    }
}

/* Test 4: Mixed SIMD and non-SIMD constructs */
void test_mixed_simt(int *out, const int *in1, const int *in2, int dim1, int dim2) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: out[0:dim1*dim2]) map(to: in1[0:dim1*dim2], in2[0:dim1*dim2])
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            
            /* Switch-like conditional structure */
            int cond = (i + j * 3) % 8;
            switch (cond) {
                case 0:
                case 1:
                    out[idx] = in1[idx] + in2[idx];
                    break;
                case 2:
                case 3:
                    out[idx] = in1[idx] - in2[idx];
                    break;
                case 4:
                case 5:
                    out[idx] = in1[idx] * in2[idx];
                    break;
                default:
                    out[idx] = in1[idx] / (in2[idx] + 1);
                    break;
            }
            
            /* SIMD reduction-like operation */
            int sum = 0;
            #pragma omp simd reduction(+:sum)
            for (int k = 0; k < 8; k++) {
                sum += (out[idx] >> k) & 1;
            }
            out[idx] ^= sum;
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
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    printf("Testing OpenMP SIMT transformation with n=%d, m=%d\n", n, m);
    
    /* Allocate and initialize arrays with pattern-based data */
    size_t total_size = n * m;
    
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(n * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * 2 * sizeof(float));
    
    double *D = (double *)malloc(total_size * sizeof(double));
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        C[i] = (i * 7) % 113;
        indices[i] = (i * 5) % (total_size * 2);
        X[i] = (float)(i % 59) * 0.5f;
        D[i] = (double)(i % 71) * 0.25;
    }
    
    for (int i = 0; i < total_size * 2; i++) {
        Y[i] = (float)(i % 83) * 0.3f;
    }
    
    for (int i = 0; i < n; i++) {
        mask[i] = (i * 11) % 19;
    }
    
    printf("Initial checksums:\n");
    printf("  A: %lu\n", compute_checksum(A, total_size * sizeof(int)));
    printf("  B: %lu\n", compute_checksum(B, total_size * sizeof(int)));
    printf("  X: %lu\n", compute_checksum(X, total_size * sizeof(float)));
    
    /* Execute test functions with different OpenMP constructs */
    printf("\nExecuting test functions...\n");
    
    test_simt_nested(A, B, C, n, m);
    printf("  test_simt_nested completed\n");
    
    test_simt_mapped(X, Y, indices, total_size, 2);
    printf("  test_simt_mapped completed\n");
    
    test_simt_conditional(D, mask, n, m);
    printf("  test_simt_conditional completed\n");
    
    test_mixed_simt(C, A, B, n, m);
    printf("  test_mixed_simt completed\n");
    
    /* Compute final checksums */
    printf("\nFinal checksums:\n");
    unsigned long checksum_A = compute_checksum(A, total_size * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, total_size * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, total_size * sizeof(double));
    unsigned long checksum_C = compute_checksum(C, total_size * sizeof(int));
    
    printf("  A: %lu\n", checksum_A);
    printf("  X: %lu\n", checksum_X);
    printf("  D: %lu\n", checksum_D);
    printf("  C: %lu\n", checksum_C);
    
    /* Verify changes occurred */
    if (checksum_A == CHECKSUM_SEED) {
        printf("WARNING: Array A unchanged - offloading may not have executed\n");
    }
    
    /* Cleanup */
    free(A);
    free(B);
    free(C);
    free(indices);
    free(mask);
    free(X);
    free(Y);
    free(D);
    
    printf("\nTest completed successfully\n");
    return 0;
}
