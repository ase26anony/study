#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m]) \
        num_teams(32) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Complex conditional that depends on loop indices
            // This should trigger the conditional structure in SIMT transformation
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2;
            } else if ((i * j) % 5 == 0) {
                A[idx] = B[idx] + C[idx];
            } else {
                A[idx] = B[idx] - C[idx];
            }
            
            // Additional control flow with thread index
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test 2: Pointer-based indirect accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32)
    for (int i = 0; i < size; i += stride) {
        // Indirect access pattern - may influence SIMT transformation for memory coalescing
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            // Complex expression with multiple operations
            X[i] = Y[idx] * 2.5f + sinf((float)i * 0.1f);
            
            // Conditional based on computed value
            if (X[i] > 100.0f) {
                X[i] = 100.0f;
            } else if (X[i] < -100.0f) {
                X[i] = -100.0f;
            }
        }
    }
}

/* Test 3: Nested parallel region with SIMD inside target */
void test_simt_conditional(double *D, int *mask, int n, int block_size) {
    #pragma omp target map(to: n, block_size) map(tofrom: D[0:n]) map(to: mask[0:n]) \
        num_teams(16)
    {
        #pragma omp parallel for simd
        for (int i = 0; i < n; i++) {
            // Condition that depends on thread index and loop index
            int tid = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            if ((tid % 2) == 0) {
                D[i] = D[i] * 2.0 + (double)team_id;
            } else {
                D[i] = D[i] / 2.0 - (double)team_id;
            }
            
            // Additional conditional based on mask
            if (mask[i] > 0) {
                D[i] = sqrt(fabs(D[i]));
            }
        }
    }
}

/* Test 4: Multiple SIMD clauses with different constructs */
void test_simt_mixed(int *out, const int *in1, const int *in2, int dim1, int dim2) {
    // First target region with distribute parallel for simd
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: dim1, dim2) map(tofrom: out[0:dim1*dim2]) \
        map(to: in1[0:dim1*dim2], in2[0:dim1*dim2]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            out[idx] = in1[idx] + in2[idx];
            
            // Nested conditionals
            if (i > j) {
                out[idx] *= 2;
                if (out[idx] > 1000) {
                    out[idx] = 1000;
                }
            }
        }
    }
    
    // Second target region with different construct
    #pragma omp target teams distribute simd \
        map(to: dim1, dim2) map(tofrom: out[0:dim1*dim2]) \
        map(to: in1[0:dim1*dim2])
    for (int i = 0; i < dim1 * dim2; i++) {
        if (i % 3 == 0) {
            out[i] = in1[i] - out[i];
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *C, int *indices, float *X, float *Y, 
                 double *D, int *mask, int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        indices[i] = (i * 7) % total_size;
        X[i] = (float)(i % 100) * 0.5f;
        Y[i] = (float)((i + 1) % 100) * 0.3f;
        D[i] = (double)(i % 50);
        mask[i] = (i % 5 == 0) ? 1 : 0;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int total_size) {
    long long checksum = 0;
    #pragma omp parallel for simd reduction(+:checksum)
    for (int i = 0; i < total_size; i++) {
        checksum += (long long)A[i];
        checksum += (long long)(X[i] * 100);
        checksum += (long long)(D[i] * 100);
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int n = 512;
    int m = 256;
    int iterations = 10;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    if (n <= 0) n = 512;
    if (m <= 0) m = 256;
    if (iterations <= 0) iterations = 10;
    
    int total_size = n * m;
    if (total_size > MAX_SIZE) {
        total_size = MAX_SIZE;
        n = m = (int)sqrt(MAX_SIZE);
    }
    
    printf("Running SIMT tests with n=%d, m=%d, total_size=%d, iterations=%d\n", 
           n, m, total_size, iterations);
    
    // Allocate arrays
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    double *D = (double *)malloc(total_size * sizeof(double));
    int *mask = (int *)malloc(total_size * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    long long total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        // Initialize arrays with pattern
        init_arrays(A, B, C, indices, X, Y, D, mask, total_size);
        
        // Execute all test functions
        test_simt_nested(A, B, C, n, m);
        
        test_simt_mapped(X, Y, indices, total_size, 2);
        
        test_simt_conditional(D, mask, total_size, 32);
        
        // Create additional arrays for mixed test
        int *out = (int *)malloc(total_size * sizeof(int));
        if (out) {
            #pragma omp parallel for simd
            for (int i = 0; i < total_size; i++) {
                out[i] = 0;
            }
            test_simt_mixed(out, B, C, n, m);
            
            // Merge results
            #pragma omp parallel for simd
            for (int i = 0; i < total_size; i++) {
                A[i] += out[i];
            }
            free(out);
        }
        
        // Compute and accumulate checksum
        long long iter_checksum = compute_checksum(A, X, D, total_size);
        total_checksum += iter_checksum;
        
        printf("  Iteration checksum: %lld\n", iter_checksum);
    }
    
    printf("Total checksum across all iterations: %lld\n", total_checksum);
    
    // Cleanup
    free(A);
    free(B);
    free(C);
    free(indices);
    free(X);
    free(Y);
    free(D);
    free(mask);
    
    return 0;
}
