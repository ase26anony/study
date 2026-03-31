#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_SIZE 10000

/* Test function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            // Conditional execution based on thread index and loop indices
            // This should trigger the conditional structure in SIMT transformation
            if ((omp_get_thread_num() % 4) == (idx % 4)) {
                A[idx] = B[idx] * 2 + i;
            } else {
                A[idx] = B[idx] / 2 + j;
            }
            
            // Additional conditional to create more complex control flow
            if (j % 8 == 0) {
                A[idx] += (i % 3) * 7;
            }
        }
    }
}

/* Test function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, int *indices, float *Y, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, indices[0:size]) map(tofrom: X[0:size], Y[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i++) {
        // Indirect access pattern - may influence SIMT transformation decisions
        int idx = indices[i] % size;
        
        // Complex conditional chain
        if (idx % 2 == 0) {
            X[i] = Y[idx] * 3.14f + (float)i;
        } else if (idx % 3 == 0) {
            X[i] = Y[idx] * 2.71f - (float)i;
        } else {
            X[i] = Y[idx] * 1.618f;
        }
        
        // Nested condition based on thread index
        if (omp_get_thread_num() % 8 < 4) {
            X[i] += 1.0f;
        }
    }
}

/* Test function 3: Separate parallel and SIMD regions with conditional */
void test_simt_conditional(int *data, int *mask, int len) {
    #pragma omp target teams distribute parallel for \
        map(to: len, mask[0:len]) map(tofrom: data[0:len])
    for (int i = 0; i < len; i++) {
        // Inner SIMD region with conditional
        #pragma omp simd
        for (int j = 0; j < 16; j++) {
            int pos = i * 16 + j;
            if (pos < len) {
                // Condition depending on both thread and SIMD lane
                if ((omp_get_thread_num() % 2) == (j % 2)) {
                    data[pos] = mask[pos] ? data[pos] * 3 : data[pos] + 1;
                } else {
                    data[pos] = mask[pos] ? data[pos] / 2 : data[pos] - 1;
                }
            }
        }
    }
}

/* Test function 4: Multiple SIMD clauses with reduction */
void test_simt_reduction(int *A, int *B, int n) {
    int sum = 0;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: n, B[0:n]) map(tofrom: A[0:n]) reduction(+:sum) \
        num_teams(32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        // Complex conditional with thread-dependent behavior
        int tid = omp_get_thread_num();
        int team = omp_get_team_num();
        
        if ((tid % 4) == 0) {
            A[i] = B[i] + team * 100;
        } else if ((tid % 4) == 1) {
            A[i] = B[i] - team * 50;
        } else {
            A[i] = B[i] * (tid % 8 + 1);
        }
        
        // Conditional reduction update
        if (A[i] > 0) {
            sum += A[i] % 100;
        }
    }
    
    // Use sum to prevent dead code elimination
    A[0] += sum % 1000;
}

/* Helper function to initialize arrays with pattern */
void init_arrays(int *A, int *B, int *mask, float *X, float *Y, int *indices, int size) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        A[i] = i % 97;
        B[i] = (i * 3) % 113;
        mask[i] = (i % 7 == 0) ? 1 : 0;
        X[i] = (float)(i % 59) * 0.1f;
        Y[i] = (float)(i % 73) * 0.2f;
        indices[i] = (i * 5) % size;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *data, int size) {
    long long sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments for sizes
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int m = (argc > 2) ? atoi(argv[2]) : 200;
    int total_size = n * m;
    
    if (total_size > MAX_SIZE) {
        printf("Size too large, reducing to %d\n", MAX_SIZE);
        total_size = MAX_SIZE;
        n = 100;
        m = 100;
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d, total=%d\n", n, m, total_size);
    
    // Dynamic allocation
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    int *mask = (int*)malloc(total_size * sizeof(int));
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size * sizeof(float));
    int *indices = (int*)malloc(total_size * sizeof(int));
    
    if (!A || !B || !mask || !X || !Y || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    init_arrays(A, B, mask, X, Y, indices, total_size);
    
    // Execute test functions with different OpenMP constructs
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, n, m);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, indices, Y, total_size);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(A, mask, total_size);
    
    printf("Running test_simt_reduction...\n");
    test_simt_reduction(B, A, total_size);
    
    // Compute and print checksums
    long long checksum_A = compute_checksum(A, total_size);
    long long checksum_B = compute_checksum(B, total_size);
    
    printf("Checksum A: %lld\n", checksum_A);
    printf("Checksum B: %lld\n", checksum_B);
    
    // Cleanup
    free(A);
    free(B);
    free(mask);
    free(X);
    free(Y);
    free(indices);
    
    return 0;
}
