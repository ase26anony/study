#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 100000

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(64) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Complex conditional that depends on both indices
            // This should trigger control flow generation
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i * j) % 5 == 0) {
                A[idx] = B[idx] / 2 + C[idx];
            } else {
                A[idx] = B[idx] + C[idx];
            }
            
            // Additional conditional with thread-specific behavior
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test 2: Pointer indirection with SIMD and safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32)
    for (int i = 0; i < size; i++) {
        int idx = indices[i];
        // Complex pointer arithmetic and conditional
        if (idx >= 0 && idx < size) {
            X[i] = Y[idx] * 2.0f;
            // Nested condition to increase control flow complexity
            if (i % 8 == 0) {
                X[i] += sinf((float)i) * 0.5f;
            }
        } else {
            X[i] = 0.0f;
        }
        
        // SIMD-unfriendly operation to potentially trigger special handling
        if (X[i] > 100.0f) {
            X[i] = 100.0f;
        }
    }
}

/* Test 3: Separate parallel and SIMD regions with thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int n) {
    #pragma omp target map(to: n) map(tofrom: D[0:n]) map(to: mask[0:n]) \
        num_teams(32)
    {
        #pragma omp parallel for simd
        for (int i = 0; i < n; i++) {
            // Condition that depends on thread number - forces divergence
            int tid = omp_get_thread_num();
            if (tid % 2 == 0) {
                D[i] = D[i] * 2.0 + mask[i];
            } else {
                D[i] = D[i] / 2.0 - mask[i];
            }
            
            // Additional nested condition
            if (mask[i] > 0 && D[i] < 0) {
                D[i] = -D[i];
            }
        }
    }
}

/* Test 4: Mixed directives to explore different lowering paths */
void test_mixed_constructs(int *out, int *in1, int *in2, int len) {
    // First a target teams distribute
    #pragma omp target teams distribute \
        map(to: len) map(tofrom: out[0:len]) map(to: in1[0:len], in2[0:len])
    for (int i = 0; i < len; i++) {
        out[i] = in1[i] + in2[i];
    }
    
    // Then a target parallel for simd
    #pragma omp target parallel for simd \
        map(to: len) map(tofrom: out[0:len]) map(to: in1[0:len])
    for (int i = 0; i < len; i++) {
        if (i % 16 == 0) {
            out[i] = out[i] * in1[i];
        } else {
            out[i] = out[i] / (in1[i] + 1);
        }
    }
}

/* Test 5: Complex nested loops with variable bounds */
void test_variable_bounds(int *result, int *data, int outer, int *inner_sizes) {
    #pragma omp target teams distribute parallel for simd \
        map(to: outer, inner_sizes[0:outer]) \
        map(tofrom: result[0:outer*100]) map(to: data[0:outer*100])
    for (int i = 0; i < outer; i++) {
        int inner = inner_sizes[i % outer];
        for (int j = 0; j < inner; j++) {
            int idx = i * 100 + j;
            // Complex condition with multiple branches
            switch (data[idx] % 4) {
                case 0:
                    result[idx] = data[idx] * 2;
                    break;
                case 1:
                    result[idx] = data[idx] + 100;
                    break;
                case 2:
                    result[idx] = data[idx] - 50;
                    break;
                default:
                    result[idx] = data[idx] / 2;
                    break;
            }
            
            // Thread-dependent operation
            if (omp_get_thread_num() % 8 < 4) {
                result[idx] += i;
            } else {
                result[idx] += j;
            }
        }
    }
}

/* Initialize arrays with pattern */
void init_arrays(int *A, int *B, int *C, int *indices, 
                 float *X, float *Y, double *D, int *mask, int size) {
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        indices[i] = (i * 7) % size;
        X[i] = (float)(i % 100) * 0.1f;
        Y[i] = (float)((i + 1) % 100) * 0.2f;
        D[i] = (double)(i % 50) * 0.5;
        mask[i] = (i % 3 == 0) ? 1 : 0;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int size) {
    long long sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < size; i++) {
        sum += A[i];
        sum += (long long)(X[i] * 100);
        sum += (long long)(D[i] * 100);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 512;
    int m = 256;
    int size = n * m;
    
    // Parse command line arguments for flexibility
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        size = n * m;
        if (size > MAX_SIZE) size = MAX_SIZE;
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d, size=%d\n", n, m, size);
    
    // Dynamic allocation
    int *A = (int *)malloc(size * sizeof(int));
    int *B = (int *)malloc(size * sizeof(int));
    int *C = (int *)malloc(size * sizeof(int));
    int *indices = (int *)malloc(size * sizeof(int));
    float *X = (float *)malloc(size * sizeof(float));
    float *Y = (float *)malloc(size * sizeof(float));
    double *D = (double *)malloc(size * sizeof(double));
    int *mask = (int *)malloc(size * sizeof(int));
    int *inner_sizes = (int *)malloc(n * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask || !inner_sizes) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize inner_sizes with variable bounds
    for (int i = 0; i < n; i++) {
        inner_sizes[i] = 50 + (i % 50);
    }
    
    // Initialize all arrays
    init_arrays(A, B, C, indices, X, Y, D, mask, size);
    
    printf("Starting OpenMP target offload tests...\n");
    
    // Execute all test functions
    test_simt_nested(A, B, C, n, m);
    
    test_simt_mapped(X, Y, indices, size);
    
    test_simt_conditional(D, mask, size);
    
    // Additional arrays for mixed tests
    int *out = (int *)malloc(size * sizeof(int));
    int *in1 = (int *)malloc(size * sizeof(int));
    int *in2 = (int *)malloc(size * sizeof(int));
    
    if (out && in1 && in2) {
        #pragma omp parallel for simd
        for (int i = 0; i < size; i++) {
            out[i] = 0;
            in1[i] = i % 73;
            in2[i] = i % 89;
        }
        
        test_mixed_constructs(out, in1, in2, size);
        
        // Use results
        #pragma omp parallel for simd
        for (int i = 0; i < size; i++) {
            A[i] += out[i] % 100;
        }
        
        free(out);
        free(in1);
        free(in2);
    }
    
    // Test with variable bounds
    int *result = (int *)malloc(n * 100 * sizeof(int));
    int *data = (int *)malloc(n * 100 * sizeof(int));
    
    if (result && data) {
        #pragma omp parallel for simd
        for (int i = 0; i < n * 100; i++) {
            result[i] = 0;
            data[i] = i % 127;
        }
        
        test_variable_bounds(result, data, n, inner_sizes);
        
        // Incorporate results
        #pragma omp parallel for simd
        for (int i = 0; i < size && i < n * 100; i++) {
            A[i] += result[i] % 50;
        }
        
        free(result);
        free(data);
    }
    
    // Compute and print checksum
    long long checksum = compute_checksum(A, X, D, size);
    printf("Final checksum: %lld\n", checksum);
    
    // Verify some values
    int verify_count = 0;
    #pragma omp parallel for reduction(+:verify_count)
    for (int i = 0; i < size; i += size/10) {
        if (A[i] != 0) verify_count++;
    }
    printf("Non-zero elements in sample: %d/10\n", verify_count);
    
    // Cleanup
    free(A); free(B); free(C); free(indices);
    free(X); free(Y); free(D); free(mask);
    free(inner_sizes);
    
    printf("Test completed successfully.\n");
    
    return 0;
}
