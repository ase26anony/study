#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 100000

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        num_teams(n/32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Conditional inside SIMD loop - may trigger SIMT transformation
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + iter;
            } else if ((i + j) % 5 == 0) {
                A[idx] = B[idx] / 2 + iter;
            } else {
                A[idx] = B[idx] + iter;
            }
            
            // Additional control flow with thread/team info
            int tid = omp_get_thread_num();
            int team = omp_get_team_num();
            if (tid % 4 == 0) {
                A[idx] += team;
            }
        }
    }
}

/* Test 2: Complex pointer access patterns with SIMD */
void test_simt_mapped(float *X, int *indices, float *Y, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: indices[0:size], Y[0:size*stride]) \
        safelen(32)
    for (int i = 0; i < size; i++) {
        // Indirect access pattern - encourages SIMT for coalescing
        int idx = indices[i];
        if (idx >= 0 && idx < size * stride) {
            // Multiple conditional paths
            if (i % 8 == 0) {
                X[i] = Y[idx] * 2.0f;
            } else if (i % 8 == 1) {
                X[i] = Y[idx] / 2.0f;
            } else if (i % 8 == 2) {
                X[i] = sqrtf(fabsf(Y[idx]));
            } else {
                X[i] = Y[idx] + (float)i;
            }
            
            // Thread-dependent computation
            int tid = omp_get_thread_num();
            if (tid % 3 == 0) {
                X[i] += 0.5f;
            }
        }
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int nrows, int ncols) {
    #pragma omp target map(tofrom: D[0:nrows*ncols]) map(to: mask[0:nrows*ncols]) \
        num_teams(nrows/16)
    {
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            #pragma omp for simd collapse(2) nowait
            for (int i = 0; i < nrows; i++) {
                for (int j = 0; j < ncols; j++) {
                    int idx = i * ncols + j;
                    
                    // Complex conditional based on thread ID and mask
                    if (mask[idx] > 0) {
                        if (tid % 2 == 0) {
                            D[idx] = sin(D[idx]) * 2.0;
                        } else {
                            D[idx] = cos(D[idx]) * 0.5;
                        }
                        
                        // Nested condition
                        if ((i + j) % 7 == 0) {
                            D[idx] += (double)tid * 0.01;
                        }
                    } else {
                        D[idx] = D[idx] * 0.9;
                    }
                }
            }
        }
    }
}

/* Test 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int size, int offset) {
    // First a simple target teams
    #pragma omp target teams distribute map(tofrom: data[0:size]) \
        num_teams(size/64)
    for (int i = 0; i < size; i++) {
        data[i] += offset;
    }
    
    // Then a parallel for simd
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:size]) simdlen(16)
    for (int i = 0; i < size; i++) {
        if (data[i] % 11 == 0) {
            data[i] *= 2;
        } else if (data[i] % 13 == 0) {
            data[i] /= 2;
        }
    }
}

/* Helper to initialize arrays */
void init_arrays(int *A, int *B, float *X, float *Y, double *D, int *mask, 
                 int *indices, int total_size, int stride) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = i % 97;
        B[i] = (i * 3) % 101;
        X[i] = (float)(i % 73) * 0.1f;
        Y[i] = (float)(i % 59) * 0.2f;
        D[i] = (double)(i % 83) * 0.01;
        mask[i] = (i % 7 == 0) ? 1 : 0;
        indices[i] = (i * stride) % (total_size * stride);
    }
}

int main(int argc, char *argv[]) {
    int n = 1000, m = 200;
    int stride = 4;
    
    // Parse command line arguments
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    if (argc >= 4) {
        stride = atoi(argv[3]);
    }
    
    int total_size = n * m;
    int total_size_with_stride = total_size * stride;
    
    printf("Testing SIMT transformation with n=%d, m=%d, total=%d\n", 
           n, m, total_size);
    
    // Dynamic allocation
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size_with_stride * sizeof(float));
    double *D = (double*)malloc(total_size * sizeof(double));
    int *mask = (int*)malloc(total_size * sizeof(int));
    int *indices = (int*)malloc(total_size * sizeof(int));
    int *data = (int*)malloc(total_size * sizeof(int));
    
    if (!A || !B || !X || !Y || !D || !mask || !indices || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    init_arrays(A, B, X, Y, D, mask, indices, total_size, stride);
    memcpy(data, A, total_size * sizeof(int));
    
    // Execute test functions multiple times
    for (int iter = 0; iter < 3; iter++) {
        printf("Iteration %d:\n", iter + 1);
        
        // Test 1: Nested loops with collapse
        test_simt_nested(A, B, n, m, iter);
        
        // Test 2: Complex pointer access
        test_simt_mapped(X, indices, Y, total_size, stride);
        
        // Test 3: Conditional execution
        test_simt_conditional(D, mask, n, m);
        
        // Test 4: Mixed constructs
        test_mixed_constructs(data, total_size, iter * 10);
    }
    
    // Compute checksums
    long long checksum_A = 0, checksum_X = 0;
    double checksum_D = 0.0;
    
    #pragma omp parallel for reduction(+:checksum_A, checksum_X, checksum_D)
    for (int i = 0; i < total_size; i++) {
        checksum_A += A[i];
        checksum_X += (long long)(X[i] * 1000);
        checksum_D += D[i];
    }
    
    printf("\nFinal checksums:\n");
    printf("Array A: %lld\n", checksum_A);
    printf("Array X: %lld\n", checksum_X);
    printf("Array D: %f\n", checksum_D);
    
    // Cleanup
    free(A); free(B); free(X); free(Y);
    free(D); free(mask); free(indices); free(data);
    
    return 0;
}
