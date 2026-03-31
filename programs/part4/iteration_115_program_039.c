#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional execution inside SIMD loop - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + iter;
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] / 2 + iter;
            } else {
                A[idx] = B[idx] + i - j + iter;
            }
            
            /* Additional control flow with thread index */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i++) {
        /* Indirect access pattern - may influence SIMT memory coalescing decisions */
        int idx = indices[i] % size;
        float temp = Y[idx] * scale;
        
        /* Conditional store with math operations */
        if (temp > 0.5f) {
            X[i] = sqrtf(temp) + sinf((float)i * 0.01f);
        } else {
            X[i] = temp * temp + cosf((float)i * 0.01f);
        }
        
        /* Thread-dependent operation */
        if (omp_get_thread_num() % 8 < 4) {
            X[i] += 0.1f;
        }
    }
}

/* Test 3: Nested parallel region with SIMD inside target */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(8)
    {
        #pragma omp parallel for simd collapse(2) \
            simdlen(8) aligned(D:64) aligned(mask:64)
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r * cols + c;
                
                /* Complex conditional based on multiple factors */
                int tid = omp_get_thread_num();
                int team = omp_get_team_num();
                
                if (mask[idx] > 0) {
                    if ((tid + team) % 2 == 0) {
                        D[idx] = exp(D[idx] * 0.5) + (double)(r - c);
                    } else {
                        D[idx] = log(fabs(D[idx]) + 1.0) + (double)(c - r);
                    }
                } else {
                    D[idx] = (double)((r * 31 + c * 17) % 100) * 0.01;
                }
                
                /* Additional SIMD-width dependent operation */
                if (idx % 16 < 8) {
                    D[idx] *= 1.01;
                }
            }
        }
    }
}

/* Test 4: Multiple SIMD constructs with different clauses */
void test_mixed_simd(int *out, const int *in1, const int *in2, int len) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: out[0:len]) map(to: in1[0:len], in2[0:len]) \
        reduction(+:out[:len]) num_teams(4)
    for (int i = 0; i < len; i++) {
        /* SIMD loop with reduction-like pattern */
        out[i] = in1[i] + in2[i];
        
        /* Conditional that might create divergent execution */
        int lane = omp_get_thread_num() % 32;
        if (lane < 16) {
            out[i] += lane;
        } else {
            out[i] -= (lane - 16);
        }
        
        /* Nested conditional */
        if (i % 7 == 0) {
            out[i] *= 2;
        } else if (i % 7 == 3) {
            out[i] /= 2;
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *indices, float *X, float *Y, 
                 double *D, int *mask, int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = (i * 17) % 97;
        indices[i] = (i * 23) % total_size;
        X[i] = (float)(i % 101) * 0.01f;
        Y[i] = (float)(i % 73) * 0.02f;
        D[i] = (double)(i % 59) * 0.03;
        mask[i] = (i % 5 == 0) ? 1 : 0;
    }
}

/* Compute checksum for verification */
unsigned long long compute_checksum(int *A, float *X, double *D, int total_size) {
    unsigned long long checksum = 0;
    
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < total_size; i++) {
        checksum += (unsigned int)A[i];
        checksum += (unsigned int)(X[i] * 1000.0f);
        checksum += (unsigned long long)(D[i] * 1000.0);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int n = 256, m = 128;  // Default sizes
    int iterations = 10;
    
    // Parse command line arguments
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    if (n <= 0 || m <= 0) {
        n = 256;
        m = 128;
    }
    
    int total_size = n * m;
    if (total_size > MAX_SIZE) {
        total_size = MAX_SIZE;
        n = 100;
        m = 100;
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d, iterations=%d\n", 
           n, m, iterations);
    
    // Allocate and initialize arrays
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    int *indices = (int*)malloc(total_size * sizeof(int));
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size * sizeof(float));
    double *D = (double*)malloc(total_size * sizeof(double));
    int *mask = (int*)malloc(total_size * sizeof(int));
    
    if (!A || !B || !indices || !X || !Y || !D || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(A, B, indices, X, Y, D, mask, total_size);
    
    // Execute test functions multiple times
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        // Test 1: Nested loops with collapse
        test_simt_nested(A, B, n, m, iter);
        
        // Test 2: Pointer-based accesses
        test_simt_mapped(X, Y, indices, total_size, 1.5f + iter * 0.1f);
        
        // Test 3: Nested parallel region
        test_simt_conditional(D, mask, n, m);
        
        // Test 4: Mixed SIMD constructs
        test_mixed_simd(A, B, indices, total_size);
        
        // Update mask for next iteration
        #pragma omp parallel for simd
        for (int i = 0; i < total_size; i++) {
            mask[i] = (mask[i] + iter) % 3;
        }
    }
    
    // Compute and print checksum
    unsigned long long checksum = compute_checksum(A, X, D, total_size);
    printf("Final checksum: %llu\n", checksum);
    
    // Cleanup
    free(A);
    free(B);
    free(indices);
    free(X);
    free(Y);
    free(D);
    free(mask);
    
    return 0;
}
