#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() % 3 == 0) && (i % 2 == 0)) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((j % 4 == 0) && (omp_get_team_num() % 2 == 0)) {
                A[idx] = B[idx] - C[idx] + omp_get_num_threads();
            } else {
                A[idx] = B[idx] + C[idx] + (i * j);
            }
            
            /* Additional conditional to create complex control flow */
            if (A[idx] > 1000) {
                A[idx] = A[idx] % 1000;
            }
        }
    }
}

/* Test function 2: Mapped data with pointer-based accesses and SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simd safelen(16)
    for (int i = 0; i < size; i++) {
        /* Indirect access pattern - important for SIMT transformation */
        int idx = indices[i] % size;
        
        /* Complex conditional execution */
        if (omp_get_thread_num() % 2 == 0) {
            X[i] = Y[idx] * 2.0f + sinf((float)i * 0.1f);
        } else {
            X[i] = Y[idx] * 0.5f + cosf((float)i * 0.05f);
        }
        
        /* Nested conditionals */
        if (X[i] > 1.0f) {
            X[i] = 1.0f / X[i];
        } else if (X[i] < -1.0f) {
            X[i] = -1.0f;
        }
    }
}

/* Test function 3: Target region with nested parallel for simd */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp parallel for simd collapse(2)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Conditional that depends on thread index */
                int tid = omp_get_thread_num();
                int team = omp_get_team_num();
                
                if ((tid % 4 == 0) && (mask[idx] > 0)) {
                    D[idx] = D[idx] * 2.0 + (double)(team * 100);
                } else if ((tid % 3 == 0) && (mask[idx] == 0)) {
                    D[idx] = D[idx] * 0.5 - (double)(tid);
                } else {
                    D[idx] = D[idx] + (double)(i + j) * 0.1;
                }
                
                /* Additional SIMD-friendly conditional */
                D[idx] = (D[idx] > 1000.0) ? 1000.0 : D[idx];
                D[idx] = (D[idx] < -1000.0) ? -1000.0 : D[idx];
            }
        }
    }
}

/* Test function 4: Multiple SIMD clauses with varying parameters */
void test_mixed_simd(int *out, int *in1, int *in2, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: n) map(tofrom: out[0:n]) map(to: in1[0:n], in2[0:n]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        /* Complex expression with conditional */
        int val = in1[i] + in2[i];
        
        if ((i % 8) == (omp_get_thread_num() % 8)) {
            out[i] = val * 3;
        } else if ((i % 5) == (omp_get_team_num() % 5)) {
            out[i] = val / 2;
        } else {
            out[i] = val + omp_get_num_threads();
        }
        
        /* Loop-carried dependency breaker */
        if (out[i] > 1000000) {
            out[i] = out[i] % 1000;
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *C, int *indices, 
                 float *X, float *Y, 
                 double *D, int *mask,
                 int n, int m, int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        indices[i] = (i * 7) % total_size;
        X[i] = (float)(i % 100) * 0.01f;
        Y[i] = (float)((i + 50) % 100) * 0.02f;
        D[i] = (double)(i % 200) * 0.5;
        mask[i] = (i % 3 == 0) ? 1 : 0;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int total_size) {
    long long sum = 0;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < total_size; i++) {
        sum += (long long)A[i];
        sum += (long long)(X[i] * 1000);
        sum += (long long)(D[i] * 100);
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 512, m = 256;
    int rows = 128, cols = 256;
    int total_size = n * m;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        total_size = n * m;
    }
    if (argc >= 5) {
        rows = atoi(argv[3]);
        cols = atoi(argv[4]);
    }
    
    printf("Testing SIMT transformation with sizes: n=%d, m=%d, total=%d\n", 
           n, m, total_size);
    printf("Additional test: rows=%d, cols=%d\n", rows, cols);
    
    /* Dynamic allocation */
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    int *C = (int*)malloc(total_size * sizeof(int));
    int *indices = (int*)malloc(total_size * sizeof(int));
    
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size * sizeof(float));
    
    double *D = (double*)malloc(rows * cols * sizeof(double));
    int *mask = (int*)malloc(rows * cols * sizeof(int));
    
    int *out = (int*)malloc(total_size * sizeof(int));
    int *in1 = (int*)malloc(total_size * sizeof(int));
    int *in2 = (int*)malloc(total_size * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask || !out || !in1 || !in2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(A, B, C, indices, X, Y, D, mask, n, m, total_size);
    
    /* Initialize additional arrays for mixed test */
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        out[i] = 0;
        in1[i] = i % 73;
        in2[i] = (i * 11) % 89;
    }
    
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, total_size);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, rows, cols);
    
    printf("Running test_mixed_simd...\n");
    test_mixed_simd(out, in1, in2, total_size);
    
    /* Compute and print checksum */
    long long checksum1 = compute_checksum(A, X, D, total_size);
    printf("Checksum 1: %lld\n", checksum1);
    
    /* Additional checksum for mixed test */
    long long checksum2 = 0;
    #pragma omp parallel for simd reduction(+:checksum2)
    for (int i = 0; i < total_size; i++) {
        checksum2 += (long long)out[i];
    }
    printf("Checksum 2: %lld\n", checksum2);
    
    /* Verify some values */
    printf("Sample values:\n");
    printf("A[0] = %d, A[100] = %d, A[1000] = %d\n", A[0], A[100], A[1000]);
    printf("X[0] = %.3f, X[100] = %.3f, X[1000] = %.3f\n", X[0], X[100], X[1000]);
    printf("D[0] = %.3f, D[100] = %.3f\n", D[0], D[100]);
    printf("out[0] = %d, out[100] = %d\n", out[0], out[100]);
    
    /* Free memory */
    free(A); free(B); free(C); free(indices);
    free(X); free(Y);
    free(D); free(mask);
    free(out); free(in1); free(in2);
    
    printf("Test completed successfully.\n");
    return 0;
}
