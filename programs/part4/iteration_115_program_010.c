#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 1000000007

/* Function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Conditional execution path inside SIMD loop
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2;
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] + i - j;
            } else {
                A[idx] = B[idx] / 2;
            }
            
            // Additional control flow with thread index dependency
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, int *indices, float *Y, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: indices[0:size], Y[0:size]) \
        safelen(32)
    for (int i = 0; i < size; i++) {
        // Indirect memory access pattern
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            // Conditional transformation based on value
            if (Y[idx] > 0.5f) {
                X[i] = Y[idx] * 2.0f;
            } else {
                X[i] = Y[idx] * 0.5f;
            }
            
            // Thread-dependent operation
            if (omp_get_thread_num() % 8 < 4) {
                X[i] += 0.1f;
            }
        }
    }
}

/* Function 3: Separate parallel and SIMD regions with conditional */
void test_simt_conditional(double *D, int *mask, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: D[0:n]) map(to: mask[0:n])
    for (int i = 0; i < n; i++) {
        // Nested SIMD region with conditional
        #pragma omp simd
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < n) {
                // Complex conditional based on multiple factors
                if ((mask[i] & (1 << (j % 8))) != 0) {
                    D[idx] = sin(D[idx]) * cos(D[idx]);
                } else {
                    D[idx] = sqrt(fabs(D[idx]));
                }
                
                // Thread index dependent operation
                if (omp_get_thread_num() % 2 == 0) {
                    D[idx] += 0.01;
                }
            }
        }
    }
}

/* Function 4: Mixed directives to explore different lowering paths */
void test_mixed_constructs(int *data, int n, int m) {
    // First: target teams with distribute parallel for simd
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Nested conditional to create complex CFG
            if (i > j) {
                if (data[idx] % 2 == 0) {
                    data[idx] = data[idx] * 3 + 1;
                } else {
                    data[idx] = data[idx] / 2;
                }
            } else {
                data[idx] = data[idx] + i - j;
            }
        }
    }
    
    // Second: Separate target region with nested parallelism
    #pragma omp target teams map(tofrom: data[0:n*m])
    {
        #pragma omp distribute parallel for simd
        for (int i = 0; i < n*m; i++) {
            // Different conditional pattern
            if (omp_get_thread_num() % 3 == 0) {
                data[i] = data[i] << 1;
            } else if (omp_get_thread_num() % 3 == 1) {
                data[i] = data[i] >> 1;
            } else {
                data[i] = data[i] ^ 0xFF;
            }
        }
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + arr[i]) % CHECKSUM_MOD;
    }
    return sum;
}

long long compute_float_checksum(float *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + (long long)(arr[i] * 1000)) % CHECKSUM_MOD;
    }
    return sum;
}

long long compute_double_checksum(double *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + (long long)(arr[i] * 1000)) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int n = 1000;
    int m = 200;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    printf("Testing with n=%d, m=%d\n", n, m);
    
    // Allocate and initialize arrays
    int total_int = n * m;
    int *A = (int*)malloc(total_int * sizeof(int));
    int *B = (int*)malloc(total_int * sizeof(int));
    int *indices = (int*)malloc(total_int * sizeof(int));
    int *mask = (int*)malloc(n * sizeof(int));
    
    float *X = (float*)malloc(total_int * sizeof(float));
    float *Y = (float*)malloc(total_int * sizeof(float));
    
    double *D = (double*)malloc(total_int * sizeof(double));
    
    // Initialize with pattern-based data
    for (int i = 0; i < total_int; i++) {
        A[i] = i % 97;
        B[i] = (i * 3) % 101;
        indices[i] = (i * 7) % total_int;
        X[i] = (float)(i % 100) / 100.0f;
        Y[i] = (float)((i * 11) % 100) / 100.0f;
        D[i] = (double)(i % 100) / 10.0;
    }
    
    for (int i = 0; i < n; i++) {
        mask[i] = i % 255;
    }
    
    printf("Initial checksums:\n");
    printf("  A: %lld\n", compute_checksum(A, total_int));
    printf("  B: %lld\n", compute_checksum(B, total_int));
    printf("  X: %lld\n", compute_float_checksum(X, total_int));
    printf("  D: %lld\n", compute_double_checksum(D, total_int));
    
    // Execute test functions with different OpenMP constructs
    printf("\nExecuting test functions...\n");
    
    // Test 1: Nested loops with SIMD
    test_simt_nested(A, B, n, m);
    
    // Test 2: Pointer-based accesses
    test_simt_mapped(X, indices, Y, total_int);
    
    // Test 3: Conditional execution
    test_simt_conditional(D, mask, total_int);
    
    // Test 4: Mixed constructs
    test_mixed_constructs(B, n, m);
    
    // Compute final checksums
    printf("\nFinal checksums:\n");
    printf("  A: %lld\n", compute_checksum(A, total_int));
    printf("  B: %lld\n", compute_checksum(B, total_int));
    printf("  X: %lld\n", compute_float_checksum(X, total_int));
    printf("  D: %lld\n", compute_double_checksum(D, total_int));
    
    // Cleanup
    free(A);
    free(B);
    free(indices);
    free(mask);
    free(X);
    free(Y);
    free(D);
    
    printf("\nTest completed.\n");
    
    return 0;
}
