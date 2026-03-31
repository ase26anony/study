#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Complex conditional that depends on both loop indices
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2;
            } else if ((i * j) % 5 == 1) {
                A[idx] = C[idx] + B[idx];
            } else {
                A[idx] = (i << 2) | (j & 0xF);
            }
            
            // Additional control flow with thread-dependent condition
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += tid;
            }
        }
    }
}

/* Test 2: Mapped pointers with indirect accesses and SIMD safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(16)
    for (int i = 0; i < size; i += stride) {
        // Indirect memory access pattern - encourages SIMT transformation
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            // Complex floating-point computation with conditional
            float temp = Y[idx] * 2.5f;
            if (temp > 100.0f) {
                X[i] = sqrtf(temp) + sinf(Y[idx]);
            } else {
                X[i] = logf(fabsf(temp) + 1.0f);
            }
        } else {
            X[i] = (float)i / size;
        }
        
        // Thread-index dependent operation
        if (omp_get_thread_num() % 8 < 4) {
            X[i] += 0.1f * omp_get_thread_num();
        }
    }
}

/* Test 3: Conditional execution with nested parallel for simd */
void test_simt_conditional(double *D, int *mask, int n, int block_size) {
    #pragma omp target map(tofrom: D[0:n]) map(to: mask[0:n]) \
        defaultmap(tofrom:scalar)
    {
        #pragma omp teams distribute
        for (int block = 0; block < n; block += block_size) {
            int end = (block + block_size < n) ? block + block_size : n;
            
            #pragma omp parallel for simd
            for (int i = block; i < end; i++) {
                // Complex condition based on thread number and mask
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if (mask[i] > 0) {
                    if ((tid % 2) == (team_id % 3)) {
                        D[i] = sin(D[i]) * cos((double)tid);
                    } else {
                        D[i] = exp(D[i] / (tid + 1.0));
                    }
                } else {
                    D[i] = (double)(i * tid) / (team_id + 1.0);
                }
                
                // Additional SIMD-friendly but complex computation
                for (int k = 0; k < 3; k++) {
                    D[i] += 0.01 * (k + tid % 4);
                }
            }
        }
    }
}

/* Test 4: Mixed directives to explore different lowering paths */
void test_mixed_constructs(int *data, int n, int m) {
    // First a simple target teams
    #pragma omp target teams map(tofrom: data[0:n])
    {
        int team_id = omp_get_team_num();
        #pragma omp distribute simd
        for (int i = 0; i < n; i++) {
            data[i] += team_id * 1000;
        }
    }
    
    // Then a more complex one
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: data[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            int tid = omp_get_thread_num();
            
            // Multiple conditions to encourage label generation
            if (tid % 16 == 0) {
                data[idx] = (data[idx] << 3) | 0x7;
            } else if (tid % 16 == 8) {
                data[idx] = (data[idx] >> 2) ^ 0xFF;
            } else {
                data[idx] = data[idx] * ((tid % 8) + 1);
            }
            
            // Early exit simulation
            if (data[idx] > 1000000) {
                data[idx] = 1000000;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int n = 512;
    int m = 256;
    
    // Parse command line arguments for flexibility
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (n <= 0) n = 512;
        if (m <= 0) m = 256;
        if (n * m > MAX_SIZE) {
            n = MAX_SIZE / m;
            if (n <= 0) n = 1;
        }
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d\n", n, m);
    
    // Allocate and initialize arrays with pattern-based data
    int total_size = n * m;
    
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    
    double *D = (double *)malloc(total_size * sizeof(double));
    
    // Initialize with non-trivial patterns
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        indices[i] = (i * 7) % total_size;
        mask[i] = (i % 11) > 5 ? 1 : 0;
        X[i] = (float)(i % 79) / 3.0f;
        Y[i] = (float)(i % 61) / 5.0f;
        D[i] = (double)(i % 43) / 7.0;
    }
    
    // Execute test functions with different OpenMP constructs
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, total_size, 2);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, total_size, 64);
    
    printf("Running test_mixed_constructs...\n");
    test_mixed_constructs(A, n, m);
    
    // Compute checksums to ensure all code paths executed
    long long checksum_int = 0;
    double checksum_float = 0.0;
    double checksum_double = 0.0;
    
    for (int i = 0; i < total_size; i++) {
        checksum_int += A[i];
        checksum_float += (double)X[i];
        checksum_double += D[i];
    }
    
    printf("Checksums:\n");
    printf("  Integer array: %lld\n", checksum_int);
    printf("  Float array: %f\n", checksum_float);
    printf("  Double array: %f\n", checksum_double);
    
    // Cleanup
    free(A);
    free(B);
    free(C);
    free(indices);
    free(mask);
    free(X);
    free(Y);
    free(D);
    
    printf("Test completed successfully.\n");
    
    return 0;
}
