#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 1000000007

// Test 1: Nested loops with SIMD clause and conditional execution
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n], C[0:m]) \
        num_teams(n/32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            // Complex conditional that depends on both loop indices
            // This should create control flow within SIMD lanes
            if ((i * j) % 17 < 8) {
                A[i * m + j] = B[i] + C[j] + (i * j) % 13;
            } else {
                A[i * m + j] = B[i] * C[j] - (i + j) % 11;
            }
            
            // Additional conditional with thread index dependency
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[i * m + j] += tid;
            }
        }
    }
}

// Test 2: Pointer-based indirect accesses with SIMD
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size*2], indices[0:size]) \
        simdlen(32) safelen(64)
    for (int i = 0; i < size; i += stride) {
        // Indirect access pattern - encourages SIMT transformation for coalescing
        int idx = indices[i] % size;
        float temp = Y[idx * 2] * 1.5f + Y[idx * 2 + 1] * 0.5f;
        
        // Conditional store with complex expression
        if (temp > 0.0f && idx % 3 == 0) {
            X[i] = sinf(temp) * cosf((float)i);
        } else if (temp < -1.0f) {
            X[i] = logf(fabsf(temp) + 1.0f);
        } else {
            X[i] = temp * temp;
        }
        
        // Additional SIMD lane-dependent operation
        int lane = omp_get_thread_num() % 32;
        X[i] += lane * 0.01f;
    }
}

// Test 3: Multiple nested parallel regions with SIMD
void test_simt_conditional(double *D, int *mask, int n, int block_size) {
    #pragma omp target map(tofrom: D[0:n]) map(to: mask[0:n]) \
        num_teams((n + block_size - 1) / block_size)
    {
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            #pragma omp for simd nowait
            for (int i = team_id * block_size; 
                 i < (team_id + 1) * block_size && i < n; 
                 i++) {
                // Conditional based on thread ID - creates divergence
                if (tid % 2 == 0) {
                    if (mask[i] > 0) {
                        D[i] = sqrt(D[i]) * 2.0;
                    } else {
                        D[i] = D[i] * D[i] / 3.0;
                    }
                } else {
                    D[i] = sin(D[i] * 0.1) + cos(D[i] * 0.05);
                }
                
                // Nested condition with thread index
                if ((tid + i) % 8 == 0) {
                    D[i] += 1.0 / (1.0 + fabs(D[i]));
                }
            }
        }
    }
}

// Test 4: Mixed constructs to explore different lowering paths
void test_mixed_constructs(int *data, int n, int m) {
    // First a simple target teams
    #pragma omp target teams distribute map(tofrom: data[0:n*m]) \
        num_teams(16)
    for (int i = 0; i < n; i++) {
        #pragma omp parallel for simd
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            data[idx] = (data[idx] * 3) % 97;
        }
    }
    
    // Then a target with parallel for simd
    #pragma omp target parallel for simd map(tofrom: data[0:n*m]) \
        num_threads(128)
    for (int i = 0; i < n * m; i++) {
        if (i % 7 == 0) {
            data[i] = data[i] + omp_get_thread_num();
        } else if (i % 13 == 0) {
            data[i] = data[i] - omp_get_team_num();
        }
    }
}

// Compute checksum to verify execution
unsigned long long compute_checksum(int *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + arr[i]) % CHECKSUM_MOD;
    }
    return sum;
}

unsigned long long compute_float_checksum(float *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        int val = (int)(arr[i] * 1000);
        sum = (sum * 31 + val) % CHECKSUM_MOD;
    }
    return sum;
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
    
    printf("Running with n=%d, m=%d, iterations=%d\n", n, m, iterations);
    
    // Allocate and initialize arrays
    int total_size = n * m;
    
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(n * sizeof(int));
    int *C = (int *)malloc(m * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * 2 * sizeof(float));
    
    double *D = (double *)malloc(total_size * sizeof(double));
    
    // Initialize with pattern-based data
    for (int i = 0; i < n; i++) {
        B[i] = i % 97;
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            A[idx] = (i * 17 + j * 13) % 197;
            indices[idx] = (i * 19 + j * 23) % total_size;
            mask[idx] = (i + j) % 5;
            X[idx] = (float)((i * 0.1 + j * 0.3)) / 10.0f;
            D[idx] = (double)((i * 0.07 + j * 0.11)) / 5.0;
        }
    }
    
    for (int i = 0; i < m; i++) {
        C[i] = (i * 29) % 89;
    }
    
    for (int i = 0; i < total_size * 2; i++) {
        Y[i] = (float)(i % 157) / 50.0f;
    }
    
    // Execute test functions multiple times
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d:\n", iter + 1);
        
        // Test 1: Nested loops with SIMD
        test_simt_nested(A, B, C, n, m, iter);
        unsigned long long checksum1 = compute_checksum(A, total_size);
        printf("  Test1 checksum: %llu\n", checksum1);
        
        // Test 2: Pointer-based indirect accesses
        test_simt_mapped(X, Y, indices, total_size, 1 + iter % 4);
        unsigned long long checksum2 = compute_float_checksum(X, total_size);
        printf("  Test2 checksum: %llu\n", checksum2);
        
        // Test 3: Conditional execution
        test_simt_conditional(D, mask, total_size, 64);
        
        // Test 4: Mixed constructs
        test_mixed_constructs(A, n, m);
        unsigned long long checksum4 = compute_checksum(A, total_size);
        printf("  Test4 checksum: %llu\n", checksum4);
        
        // Modify some parameters for next iteration
        for (int i = 0; i < n; i++) {
            B[i] = (B[i] + 1) % 97;
        }
    }
    
    // Final checksum
    unsigned long long final_checksum = compute_checksum(A, total_size);
    printf("\nFinal checksum: %llu\n", final_checksum);
    
    // Cleanup
    free(A);
    free(B);
    free(C);
    free(indices);
    free(mask);
    free(X);
    free(Y);
    free(D);
    
    return 0;
}
