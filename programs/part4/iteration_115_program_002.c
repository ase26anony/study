#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 5381

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            int thread_id = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            /* Complex condition that may trigger SIMT transformation */
            if ((thread_id % 3 == 0) && (i % 2 == 0) && (j % 4 == 0)) {
                A[idx] = B[idx] * C[idx] + team_id;
            } else if ((thread_id % 5 == 0) && (i > n/2)) {
                A[idx] = B[idx] - C[idx] + thread_id;
            } else {
                A[idx] = B[idx] + C[idx] + i + j;
            }
            
            /* Additional control flow with early exit simulation */
            if (A[idx] > 1000 && j < m/2) {
                A[idx] = A[idx] % 100;
            }
        }
    }
}

/* Test function 2: Mapped data with pointer-based accesses and SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern - may influence SIMT transformation */
        int idx = indices[i % size];
        
        /* Complex computation with conditional */
        if (idx >= 0 && idx < size) {
            float temp = Y[idx];
            
            /* SIMD-friendly but with control flow */
            if (temp > 0.5f) {
                X[i] = temp * temp + sinf((float)i * 0.01f);
            } else if (temp < -0.5f) {
                X[i] = temp * 0.5f + cosf((float)i * 0.02f);
            } else {
                X[i] = temp + tanf((float)i * 0.005f);
            }
            
            /* Additional condition based on thread */
            if (omp_get_thread_num() % 8 == 0) {
                X[i] = fmodf(X[i], 2.0f);
            }
        } else {
            X[i] = 0.0f;
        }
    }
}

/* Test function 3: Target region with nested parallel for simd */
void test_simt_conditional(double *D, int *mask, int n, int m) {
    #pragma omp target map(to: n, m) map(tofrom: D[0:n*m]) map(to: mask[0:n*m])
    {
        #pragma omp parallel for simd collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                
                /* Condition depending on thread index modulo operations */
                int thread_mod = omp_get_thread_num() % 7;
                int warp_like_condition = omp_get_thread_num() % 32;
                
                if (mask[idx] > 0) {
                    if (thread_mod == 0) {
                        D[idx] = D[idx] * 2.0 + (double)warp_like_condition;
                    } else if (thread_mod == 1 || thread_mod == 2) {
                        D[idx] = D[idx] / 1.5 + (double)(i * j);
                    } else {
                        D[idx] = sqrt(fabs(D[idx])) + (double)thread_mod;
                    }
                } else {
                    if (warp_like_condition < 16) {
                        D[idx] = D[idx] * 0.5;
                    } else {
                        D[idx] = D[idx] * 1.5;
                    }
                }
                
                /* SIMD lane-like condition */
                if ((j % 4) == (omp_get_thread_num() % 4)) {
                    D[idx] += 0.25;
                }
            }
        }
    }
}

/* Test function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int size, int iter) {
    /* First a simple target teams */
    #pragma omp target teams map(tofrom: data[0:size]) map(to: size, iter)
    {
        int team_id = omp_get_team_num();
        #pragma omp distribute simd
        for (int i = 0; i < size; i++) {
            data[i] += team_id * iter;
        }
    }
    
    /* Then a target with parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:size]) map(to: size)
    for (int i = 0; i < size; i++) {
        int thread_id = omp_get_thread_num();
        if (thread_id % 3 == 0) {
            data[i] = data[i] * 2 - i;
        } else if (thread_id % 3 == 1) {
            data[i] = data[i] / 2 + i;
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
    int size = 10000;
    
    /* Parse command line arguments for flexibility */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            size = atoi(argv[3]);
        }
    }
    
    printf("Running SIMT transformation tests with n=%d, m=%d, size=%d\n", n, m, size);
    
    /* Allocate and initialize arrays with pattern-based data */
    int *A = (int *)malloc(n * m * sizeof(int));
    int *B = (int *)malloc(n * m * sizeof(int));
    int *C = (int *)malloc(n * m * sizeof(int));
    int *mask = (int *)malloc(n * m * sizeof(int));
    
    float *X = (float *)malloc(size * sizeof(float));
    float *Y = (float *)malloc(size * sizeof(float));
    int *indices = (int *)malloc(size * sizeof(int));
    
    double *D = (double *)malloc(n * m * sizeof(double));
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < n * m; i++) {
        A[i] = 0;
        B[i] = (i % 97) * 3;
        C[i] = (i % 73) * 2;
        mask[i] = (i % 5 == 0) ? 1 : 0;
        D[i] = (double)(i % 89) / 3.0;
    }
    
    for (int i = 0; i < size; i++) {
        X[i] = 0.0f;
        Y[i] = sinf((float)i * 0.1f);
        indices[i] = (i * 7) % size;
    }
    
    printf("Initial checksums:\n");
    printf("  A: %lu\n", compute_checksum(A, n * m * sizeof(int)));
    printf("  B: %lu\n", compute_checksum(B, n * m * sizeof(int)));
    printf("  X: %lu\n", compute_checksum(X, size * sizeof(float)));
    printf("  D: %lu\n", compute_checksum(D, n * m * sizeof(double)));
    
    /* Execute test functions with different OpenMP constructs */
    printf("\nExecuting test functions...\n");
    
    test_simt_nested(A, B, C, n, m);
    printf("  test_simt_nested completed\n");
    
    test_simt_mapped(X, Y, indices, size, 2);
    printf("  test_simt_mapped completed\n");
    
    test_simt_conditional(D, mask, n, m);
    printf("  test_simt_conditional completed\n");
    
    test_mixed_constructs(A, n * m / 2, 3);
    printf("  test_mixed_constructs completed\n");
    
    /* Compute final checksums */
    printf("\nFinal checksums:\n");
    printf("  A: %lu\n", compute_checksum(A, n * m * sizeof(int)));
    printf("  B: %lu\n", compute_checksum(B, n * m * sizeof(int)));
    printf("  X: %lu\n", compute_checksum(X, size * sizeof(float)));
    printf("  D: %lu\n", compute_checksum(D, n * m * sizeof(double)));
    
    /* Cleanup */
    free(A); free(B); free(C); free(mask);
    free(X); free(Y); free(indices);
    free(D);
    
    printf("\nTest completed successfully.\n");
    
    return 0;
}
