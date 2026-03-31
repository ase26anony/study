#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_SIZE 10000

/* Test function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int n, int m, float *A, float *B, int *C) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(32) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional execution inside SIMD loop - may trigger SIMT transformation */
            if ((i + j) % 8 == 0) {
                A[idx] = B[idx] * 2.0f;
            } else if ((i + j) % 5 == 0) {
                A[idx] = B[idx] + C[idx];
            } else {
                A[idx] = B[idx] - C[idx];
            }
            
            /* Additional control flow with thread index */
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += 1.0f;
            }
        }
    }
}

/* Test function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(int n, float *A, float *B, float *C, int *indices) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: A[0:n]) map(to: B[0:n], C[0:n], indices[0:n]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < n; i++) {
        /* Indirect memory access pattern - encourages SIMT transformation */
        int idx = indices[i] % n;
        float temp = B[idx] * C[i];
        
        /* Conditional store with thread-dependent behavior */
        if (omp_get_thread_num() % 3 == 0) {
            A[i] = temp * 0.5f;
        } else {
            A[i] = temp * 2.0f;
        }
        
        /* Nested conditionals */
        if (i % 7 == 0 && omp_get_team_num() % 2 == 0) {
            A[i] += 100.0f;
        }
    }
}

/* Test function 3: Separate parallel and SIMD regions */
void test_simt_conditional(int n, int m, double *D, double *E, int *mask) {
    #pragma omp target teams distribute map(tofrom: D[0:n*m]) map(to: E[0:n*m], mask[0:n*m])
    for (int i = 0; i < n; i++) {
        #pragma omp parallel for simd
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Complex conditional based on multiple factors */
            if (mask[idx] > 0 && omp_get_thread_num() % 2 == 0) {
                D[idx] = E[idx] * 3.14159;
            } else if (mask[idx] < 0 || omp_get_team_num() % 3 == 0) {
                D[idx] = E[idx] / 2.71828;
            } else {
                D[idx] = E[idx] + (i * m + j);
            }
            
            /* Additional branching */
            switch (j % 4) {
                case 0: D[idx] += 1.0; break;
                case 1: D[idx] -= 1.0; break;
                case 2: D[idx] *= 1.5; break;
                case 3: D[idx] /= 1.5; break;
            }
        }
    }
}

/* Test function 4: Multiple SIMD clauses with reduction */
void test_simt_reduction(int n, float *X, float *Y, int *flags) {
    float sum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:n], Y[0:n]) map(to: flags[0:n]) \
        reduction(+:sum) collapse(2) num_teams(64)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            
            /* Thread-dependent computation */
            int tid = omp_get_thread_num();
            if (tid % 8 < 4) {
                X[idx] = Y[idx] * tid;
            } else {
                X[idx] = Y[idx] / (tid + 1);
            }
            
            /* Conditional reduction update */
            if (flags[idx] || (tid % 16 == 0)) {
                sum += X[idx];
            }
        }
    }
    
    /* Use sum to prevent dead code elimination */
    X[0] += sum / n;
}

/* Helper function to initialize arrays with patterns */
void init_arrays(int n, int m, float *A, float *B, float *C, 
                 double *D, double *E, int *indices, int *mask, int *flags) {
    #pragma omp parallel for simd
    for (int i = 0; i < n * m; i++) {
        A[i] = (i % 97) * 0.1f;
        B[i] = (i % 113) * 0.2f;
        C[i] = i % 71;
        D[i] = (i % 151) * 0.01;
        E[i] = (i % 173) * 0.02;
        indices[i] = (i * 3 + 7) % (n * m);
        mask[i] = (i % 11) - 5;
        flags[i] = (i % 19) > 10;
    }
}

/* Compute checksum to verify execution */
float compute_checksum(int n, int m, float *A, double *D) {
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < n * m; i++) {
        checksum += A[i] + (float)D[i];
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments for flexibility */
    int n = 512;
    int m = 256;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    if (n * m > MAX_SIZE) {
        printf("Size too large, using %d\n", MAX_SIZE);
        n = m = 100;  /* Conservative fallback */
    }
    
    /* Dynamic allocation with non-constant sizes */
    int total = n * m;
    float *A = (float*)malloc(total * sizeof(float));
    float *B = (float*)malloc(total * sizeof(float));
    float *C = (float*)malloc(total * sizeof(float));
    double *D = (double*)malloc(total * sizeof(double));
    double *E = (double*)malloc(total * sizeof(double));
    int *indices = (int*)malloc(total * sizeof(int));
    int *mask = (int*)malloc(total * sizeof(int));
    int *flags = (int*)malloc(total * sizeof(int));
    
    if (!A || !B || !C || !D || !E || !indices || !mask || !flags) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data */
    init_arrays(n, m, A, B, C, D, E, indices, mask, flags);
    
    printf("Starting OpenMP target tests with n=%d, m=%d\n", n, m);
    
    /* Execute test functions with different OpenMP constructs */
    test_simt_nested(n, m, A, B, indices);
    printf("Completed test_simt_nested\n");
    
    test_simt_mapped(total, C, A, B, indices);
    printf("Completed test_simt_mapped\n");
    
    test_simt_conditional(n, m, D, E, mask);
    printf("Completed test_simt_conditional\n");
    
    test_simt_reduction(n, A, B, flags);
    printf("Completed test_simt_reduction\n");
    
    /* Compute and print checksum */
    float checksum = compute_checksum(n, m, A, D);
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(A); free(B); free(C);
    free(D); free(E);
    free(indices); free(mask); free(flags);
    
    return 0;
}
