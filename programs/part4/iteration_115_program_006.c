#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <assert.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Conditional that depends on both loop indices
            if ((i + j * iter) % 4 == 0) {
                A[idx] = B[idx] * 2 + (i % 3);
            } else if ((i ^ j) & 1) {
                A[idx] = B[idx] / 2 - (j % 5);
            } else {
                A[idx] = B[idx] + (i * j) % 7;
            }
            // Additional control flow to complicate SIMD transformation
            switch ((i + j) % 3) {
                case 0: A[idx] += 1; break;
                case 1: A[idx] -= 1; break;
                case 2: A[idx] *= 2; break;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, int *indices, float *Y, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: indices[0:size], Y[0:size*stride]) \
        safelen(32) aligned(X:64, Y:64)
    for (int i = 0; i < size; i++) {
        int idx = indices[i];
        // Indirect access pattern that requires careful SIMD handling
        if (idx >= 0 && idx < size * stride) {
            float val = Y[idx];
            // Complex conditional transformation
            if (val > 0.0f) {
                X[i] = val * val + (i % 10) * 0.1f;
            } else {
                X[i] = val * 0.5f - (i % 7) * 0.01f;
            }
            
            // Additional SIMD-unfriendly pattern
            for (int k = 0; k < 3; k++) {
                X[i] += 0.001f * k * (i % (k+2));
            }
        } else {
            X[i] = 0.0f;
        }
    }
}

/* Test 3: Conditional based on thread ID within SIMD region */
void test_simt_conditional(double *C, double *D, int rows, int cols, int offset) {
    #pragma omp target map(tofrom: C[0:rows*cols]) map(to: D[0:rows*cols]) \
        num_teams(8)
    {
        #pragma omp parallel for simd collapse(2) \
            reduction(+:offset) private(offset)
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int tid = omp_get_thread_num();
                int idx = r * cols + c;
                
                // SIMT transformation trigger: condition depends on thread ID
                if (tid % 4 == 0) {
                    C[idx] = D[idx] * 2.0 + (tid % 8) * 0.125;
                } else if (tid % 4 == 1) {
                    C[idx] = D[idx] / 2.0 - (tid % 6) * 0.1;
                } else if (tid % 4 == 2) {
                    C[idx] = D[idx] + sin((double)(tid % 16) * 0.1);
                } else {
                    C[idx] = D[idx] - cos((double)(tid % 12) * 0.05);
                }
                
                // Additional thread-dependent computation
                int lane = tid % 32;
                if (lane < 16) {
                    C[idx] += (lane * 0.01) * (r % 4);
                } else {
                    C[idx] -= (lane * 0.005) * (c % 3);
                }
            }
        }
    }
}

/* Test 4: Mixed directives to explore different lowering paths */
void test_mixed_simd(int *E, int *F, int *G, int dim1, int dim2, int dim3) {
    // First target region with distribute parallel for simd
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: E[0:dim1*dim2]) map(to: F[0:dim1*dim2]) \
        num_teams(dim1/16) thread_limit(256)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            E[idx] = F[idx] + (i * j) % 11;
        }
    }
    
    // Second target region with separate parallel and simd
    #pragma omp target teams distribute map(tofrom: G[0:dim2*dim3]) \
        num_teams(dim2/8)
    for (int j = 0; j < dim2; j++) {
        #pragma omp parallel for simd \
            num_threads(64)
        for (int k = 0; k < dim3; k++) {
            int idx = j * dim3 + k;
            // Complex condition to trigger SIMT branching
            if ((j + k) % 7 == 0) {
                G[idx] = E[j] * 3 - (k % 5);
            } else if ((j * k) % 13 == 0) {
                G[idx] = E[j] / 2 + (k % 3);
            } else {
                G[idx] = E[j] + (j ^ k) % 17;
            }
        }
    }
}

/* Helper function to compute checksum */
int compute_checksum(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + data[i]) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int base_size = 256;
    int iterations = 10;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    // Dynamic allocations with varying sizes
    int n = base_size;
    int m = base_size / 2;
    int size1 = n * m;
    int size2 = base_size * 64;
    int rows = base_size / 4;
    int cols = base_size / 4;
    
    int *A = (int*)malloc(size1 * sizeof(int));
    int *B = (int*)malloc(size1 * sizeof(int));
    float *X = (float*)malloc(size2 * sizeof(float));
    float *Y = (float*)malloc(size2 * 2 * sizeof(float));
    int *indices = (int*)malloc(size2 * sizeof(int));
    double *C = (double*)malloc(rows * cols * sizeof(double));
    double *D = (double*)malloc(rows * cols * sizeof(double));
    int *E = (int*)malloc(size1 * sizeof(int));
    int *F = (int*)malloc(size1 * sizeof(int));
    int *G = (int*)malloc(m * base_size * sizeof(int));
    
    // Initialize arrays with pattern-based data
    for (int i = 0; i < size1; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        E[i] = (i * 5) % 101;
        F[i] = (i * 7) % 103;
    }
    
    for (int i = 0; i < size2; i++) {
        X[i] = 0.0f;
        Y[i] = (float)((i * 11) % 199) * 0.01f;
        Y[i + size2] = (float)((i * 13) % 211) * 0.005f;
        indices[i] = (i * 17) % (size2 * 2);
    }
    
    for (int i = 0; i < rows * cols; i++) {
        C[i] = 0.0;
        D[i] = (double)((i * 19) % 307) * 0.001;
    }
    
    for (int i = 0; i < m * base_size; i++) {
        G[i] = (i * 23) % 401;
    }
    
    // Execute test functions multiple times with varying parameters
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        // Vary parameters to explore different code paths
        int current_n = n + (iter % 5) * 8;
        int current_m = m + (iter % 3) * 4;
        
        test_simt_nested(A, B, current_n, current_m, iter);
        
        int current_size = size2 / (1 + (iter % 3));
        test_simt_mapped(X, indices, Y, current_size, 2);
        
        int current_rows = rows + (iter % 2) * 8;
        int current_cols = cols + (iter % 2) * 8;
        test_simt_conditional(C, D, current_rows, current_cols, iter);
        
        test_mixed_simd(E, F, G, current_n/2, current_m, base_size/4);
    }
    
    // Compute and print checksums to ensure all code executed
    int checksum_A = compute_checksum(A, size1);
    int checksum_E = compute_checksum(E, size1);
    int checksum_G = compute_checksum(G, m * base_size);
    
    printf("Checksums:\n");
    printf("  A: %d\n", checksum_A);
    printf("  E: %d\n", checksum_E);
    printf("  G: %d\n", checksum_G);
    
    // Verify results are non-zero (indicating execution)
    assert(checksum_A != 0);
    assert(checksum_E != 0);
    assert(checksum_G != 0);
    
    // Cleanup
    free(A); free(B); free(X); free(Y); free(indices);
    free(C); free(D); free(E); free(F); free(G);
    
    printf("Test completed successfully.\n");
    return 0;
}
