#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

/* Test function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        num_teams(32) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional execution inside SIMD loop - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] - C[idx];
            } else {
                A[idx] = B[idx] + C[idx] * 3;
            }
            
            /* Additional control flow with thread index */
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: Y[0:size], indices[0:size]) map(tofrom: X[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern - may influence SIMT transformation */
        int idx = indices[i % size];
        if (idx >= 0 && idx < size) {
            X[i] = Y[idx] * 2.5f;
            
            /* Nested condition based on computed value */
            if (X[i] > 100.0f) {
                X[i] = 100.0f;
            } else if (X[i] < -100.0f) {
                X[i] = -100.0f;
            }
        }
        
        /* Additional computation with thread-dependent branching */
        int tid = omp_get_thread_num();
        if (tid % 8 < 4) {
            X[i] += tid * 0.01f;
        }
    }
}

/* Test function 3: Separate parallel and SIMD regions with conditional */
void test_simt_conditional(double *D, int *mask, int nrows, int ncols) {
    #pragma omp target map(to: mask[0:nrows*ncols]) map(tofrom: D[0:nrows*ncols]) \
        num_teams(64)
    {
        #pragma omp parallel for simd collapse(2) schedule(static)
        for (int r = 0; r < nrows; r++) {
            for (int c = 0; c < ncols; c++) {
                int idx = r * ncols + c;
                
                /* Complex conditional that depends on thread ID */
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if (mask[idx] > 0) {
                    if (tid % 2 == 0) {
                        D[idx] = D[idx] * 1.5 + team_id;
                    } else {
                        D[idx] = D[idx] * 0.5 - team_id;
                    }
                } else {
                    if ((tid + team_id) % 3 == 0) {
                        D[idx] = -D[idx];
                    }
                }
                
                /* Additional SIMD-width dependent operation */
                D[idx] += (tid % 16) * 0.1;
            }
        }
    }
}

/* Test function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *out, const int *in1, const int *in2, int len) {
    /* First a target teams region */
    #pragma omp target teams distribute parallel for \
        map(to: in1[0:len], in2[0:len]) map(tofrom: out[0:len])
    for (int i = 0; i < len; i++) {
        out[i] = in1[i] + in2[i];
    }
    
    /* Then a target simd region */
    #pragma omp target simd map(tofrom: out[0:len])
    for (int i = 0; i < len; i++) {
        if (i % 7 == 0) {
            out[i] *= 2;
        }
    }
    
    /* Finally a combined construct */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: out[0:len]) simdlen(8)
    for (int i = 0; i < len; i++) {
        /* Thread-dependent conditional */
        if (omp_get_thread_num() % 5 == 0) {
            out[i] += i;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments for dynamic sizing */
    int n = 1000;
    int m = 200;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d\n", n, m);
    
    /* Dynamically allocate arrays with pattern-based initialization */
    int total_size = n * m;
    
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    
    double *D = (double *)malloc(total_size * sizeof(double));
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        indices[i] = (i * 7) % total_size;
        mask[i] = (i % 13) > 6 ? 1 : -1;
        X[i] = (float)(i % 50);
        Y[i] = (float)((i * 2) % 75);
        D[i] = (double)(i % 100) * 0.5;
    }
    
    /* Execute test functions with different OpenMP constructs */
    test_simt_nested(A, B, C, n, m);
    
    test_simt_mapped(X, Y, indices, total_size, 2);
    
    test_simt_conditional(D, mask, n, m);
    
    /* Test with mixed constructs */
    int *out = (int *)malloc(total_size * sizeof(int));
    int *in1 = (int *)malloc(total_size * sizeof(int));
    int *in2 = (int *)malloc(total_size * sizeof(int));
    
    for (int i = 0; i < total_size; i++) {
        out[i] = 0;
        in1[i] = i % 23;
        in2[i] = (i * 5) % 29;
    }
    
    test_mixed_constructs(out, in1, in2, total_size);
    
    /* Compute checksums to ensure all code paths executed */
    long long checksum_A = 0;
    long long checksum_X = 0;
    double checksum_D = 0.0;
    long long checksum_out = 0;
    
    for (int i = 0; i < total_size; i++) {
        checksum_A += A[i];
        checksum_X += (long long)X[i];
        checksum_D += D[i];
        checksum_out += out[i];
    }
    
    printf("Checksum A: %lld\n", checksum_A);
    printf("Checksum X: %lld\n", checksum_X);
    printf("Checksum D: %f\n", checksum_D);
    printf("Checksum out: %lld\n", checksum_out);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices); free(mask);
    free(X); free(Y); free(D);
    free(out); free(in1); free(in2);
    
    return 0;
}
