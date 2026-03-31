#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <assert.h>

#define CHECKSUM_SEED 5381

/* Function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        num_teams(n/32) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Conditional execution path that depends on thread/iteration
            if ((i + j) % 4 == 0) {
                A[idx] = B[idx] * 2 + iter;
            } else if ((i + j) % 3 == 0) {
                A[idx] = B[idx] / 2 + iter;
            } else {
                A[idx] = B[idx] + i - j + iter;
            }
            
            // Additional conditional with thread-specific behavior
            if (omp_get_thread_num() % 8 < 4) {
                A[idx] += (i % 16);
            } else {
                A[idx] -= (j % 16);
            }
        }
    }
}

/* Function 2: Complex data mapping with pointer indirection */
void test_simt_mapped(float *X, int *indices, float *Y, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: indices[0:size], Y[0:size*stride]) \
        simdlen(32) safelen(64)
    for (int i = 0; i < size; i++) {
        int idx = indices[i];
        // Complex memory access pattern with bounds checking
        if (idx >= 0 && idx < size * stride) {
            float val = Y[idx];
            
            // Multiple conditional paths
            if (i % 3 == 0) {
                X[i] = val * 1.5f + (i % 100) * 0.01f;
            } else if (i % 7 == 0) {
                X[i] = val / 1.3f - (i % 50) * 0.02f;
            } else {
                X[i] = val + (idx % 1000) * 0.001f;
            }
            
            // SIMD-width dependent operation
            for (int k = 0; k < 4; k++) {
                if ((i + k) % 8 == 0) {
                    X[i] += 0.1f * k;
                }
            }
        }
    }
}

/* Function 3: Explicit parallel region with SIMD inside */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target teams distribute map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(rows/16)
    for (int i = 0; i < rows; i++) {
        #pragma omp parallel for simd \
            num_threads(64) simdlen(16) \
            reduction(+:D[i*cols])
        for (int j = 0; j < cols; j++) {
            int tid = omp_get_thread_num();
            int idx = i * cols + j;
            
            // Thread-dependent conditional execution
            if (tid % 2 == 0) {
                if (mask[idx] > 0) {
                    D[idx] = D[idx] * 2.0 + (tid % 32) * 0.01;
                } else {
                    D[idx] = D[idx] / 1.5 - (tid % 16) * 0.02;
                }
            } else {
                // Different computation path for odd threads
                D[idx] = D[idx] + (i % 8) * 0.1 - (j % 8) * 0.05;
                
                // Nested condition based on thread index
                if (tid % 4 == 1) {
                    D[idx] *= 0.9;
                } else if (tid % 4 == 3) {
                    D[idx] *= 1.1;
                }
            }
            
            // Additional SIMD-specific conditional
            if (j % 32 < 16) {
                D[idx] += 0.001 * (j % 64);
            }
        }
    }
}

/* Function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int *pattern, int n, int mode) {
    if (mode == 0) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n]) map(to: pattern[0:n]) \
            collapse(2)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                data[idx] = pattern[idx] + (i % 64) * (j % 32);
                
                // Complex conditional chain
                if (data[idx] % 7 == 0) {
                    data[idx] *= 2;
                } else if (data[idx] % 5 == 0) {
                    data[idx] /= 2;
                }
            }
        }
    } else {
        #pragma omp target teams map(tofrom: data[0:n]) num_teams(8)
        {
            #pragma omp parallel for simd simdlen(8)
            for (int i = 0; i < n; i++) {
                int tid = omp_get_thread_num();
                data[i] = pattern[i] ^ (tid << (i % 16));
                
                if (tid % 3 == 0) {
                    data[i] += i;
                }
            }
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
    // Parse command line arguments
    int base_size = 1024;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    // Calculate dimensions for nested loops
    int n = base_size;
    int m = base_size / 2;
    int total_elements = n * m;
    
    // Allocate and initialize arrays
    int *A = (int *)malloc(total_elements * sizeof(int));
    int *B = (int *)malloc(total_elements * sizeof(int));
    float *X = (float *)malloc(n * sizeof(float));
    float *Y = (float *)malloc(n * m * sizeof(float));
    int *indices = (int *)malloc(n * sizeof(int));
    double *D = (double *)malloc(total_elements * sizeof(double));
    int *mask = (int *)malloc(total_elements * sizeof(int));
    int *pattern = (int *)malloc(n * sizeof(int));
    
    assert(A && B && X && Y && indices && D && mask && pattern);
    
    // Initialize with pattern-based data
    for (int i = 0; i < total_elements; i++) {
        A[i] = 0;
        B[i] = i % 97;
        D[i] = (i % 113) * 0.01;
        mask[i] = (i % 3 == 0) ? 1 : -1;
    }
    
    for (int i = 0; i < n; i++) {
        X[i] = (i % 71) * 0.1f;
        indices[i] = (i * 7) % (n * m);
        pattern[i] = i % 255;
    }
    
    for (int i = 0; i < n * m; i++) {
        Y[i] = (i % 59) * 0.05f;
    }
    
    printf("Initial checksums:\n");
    printf("  A: %lu\n", compute_checksum(A, total_elements * sizeof(int)));
    printf("  B: %lu\n", compute_checksum(B, total_elements * sizeof(int)));
    printf("  X: %lu\n", compute_checksum(X, n * sizeof(float)));
    
    // Execute test functions multiple times with different parameters
    for (int iter = 0; iter < iterations; iter++) {
        if (iter % 4 == 0) {
            test_simt_nested(A, B, n, m, iter);
        }
        
        if (iter % 3 == 0) {
            test_simt_mapped(X, indices, Y, n, m);
        }
        
        if (iter % 2 == 0) {
            test_simt_conditional(D, mask, n, m);
        }
        
        test_mixed_constructs(pattern, indices, n, iter % 2);
    }
    
    // Final checksums
    printf("\nFinal checksums:\n");
    printf("  A: %lu\n", compute_checksum(A, total_elements * sizeof(int)));
    printf("  B: %lu\n", compute_checksum(B, total_elements * sizeof(int)));
    printf("  X: %lu\n", compute_checksum(X, n * sizeof(float)));
    printf("  D: %lu\n", compute_checksum(D, total_elements * sizeof(double)));
    printf("  pattern: %lu\n", compute_checksum(pattern, n * sizeof(int)));
    
    // Cleanup
    free(A);
    free(B);
    free(X);
    free(Y);
    free(indices);
    free(D);
    free(mask);
    free(pattern);
    
    return 0;
}
