#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

// Test 1: Nested loops with collapse and conditional inside SIMD
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter, B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        num_teams(32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Complex conditional that depends on loop indices
            // This should trigger the conditional structure in SIMT transformation
            if ((i + j * 3) % 7 == 0) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((i ^ j) % 5 == 2) {
                A[idx] = B[idx] - C[idx] * iter;
            } else {
                A[idx] = B[idx] + C[idx];
            }
            
            // Additional control flow with thread information
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += (i % 3) * (j % 5);
            }
        }
    }
}

// Test 2: Pointer-based indirect accesses with SIMD
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, indices[0:size*2], Y[0:size*stride]) \
        map(tofrom: X[0:size*stride]) \
        safelen(32) num_teams(64)
    for (int i = 0; i < size; i++) {
        // Complex pointer arithmetic and indirect access
        float *base_x = X + i * stride;
        float *base_y = Y + indices[i] * stride;
        int *idx_ptr = indices + size + i;
        
        for (int k = 0; k < stride; k++) {
            // Nested loop with SIMD pragma inside target region
            #pragma omp simd
            for (int j = 0; j < 4; j++) {
                int pos = k * 4 + j;
                if (pos < stride) {
                    // Conditional based on indirect index
                    if (indices[i] % 3 == 0) {
                        base_x[pos] = base_y[pos] * 2.0f + (float)(*idx_ptr);
                    } else {
                        base_x[pos] = base_y[pos] / 1.5f - (float)i;
                    }
                }
            }
        }
    }
}

// Test 3: Multiple nested parallel regions with SIMD
void test_simt_conditional(double *D, int *mask, int rows, int cols, int depth) {
    #pragma omp target map(to: rows, cols, depth, mask[0:rows*cols]) \
        map(tofrom: D[0:rows*cols*depth]) default(none)
    {
        #pragma omp teams num_teams(16) thread_limit(128)
        {
            #pragma omp distribute
            for (int i = 0; i < rows; i++) {
                #pragma omp parallel for simd
                for (int j = 0; j < cols; j++) {
                    int idx = i * cols + j;
                    
                    // Conditional that depends on thread number
                    int tid = omp_get_thread_num();
                    int team_id = omp_get_team_num();
                    
                    if ((tid ^ team_id) % 2 == 0) {
                        // SIMD loop with variable bounds
                        #pragma omp simd
                        for (int k = 0; k < depth; k++) {
                            int full_idx = idx * depth + k;
                            if (mask[idx] > 0) {
                                D[full_idx] = sin(D[full_idx]) * cos((double)tid);
                            } else {
                                D[full_idx] = exp(D[full_idx] * 0.5) + (double)team_id;
                            }
                        }
                    } else {
                        #pragma omp simd
                        for (int k = 0; k < depth; k++) {
                            int full_idx = idx * depth + k;
                            D[full_idx] = log(fabs(D[full_idx]) + 1.0) * (double)(tid % 8);
                        }
                    }
                }
            }
        }
    }
}

// Test 4: Mixed constructs to explore different lowering paths
void test_mixed_constructs(int *data, int size, int offset) {
    // First a simple target teams distribute
    #pragma omp target teams distribute \
        map(to: size, offset) map(tofrom: data[0:size]) \
        num_teams(8)
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] + offset) % 256;
    }
    
    // Then a parallel for simd inside target
    #pragma omp target map(to: size) map(tofrom: data[0:size])
    {
        #pragma omp parallel for simd
        for (int i = 0; i < size; i++) {
            // Complex expression with conditional
            data[i] = (data[i] * 3) ^ (i % 31);
            if (omp_get_thread_num() % 3 == 0) {
                data[i] += 1000;
            }
        }
    }
}

// Helper function to compute checksum
unsigned long compute_checksum(void *array, size_t size_bytes) {
    unsigned long checksum = 0;
    unsigned char *bytes = (unsigned char *)array;
    for (size_t i = 0; i < size_bytes; i++) {
        checksum = (checksum * 31 + bytes[i]) % CHECKSUM_MOD;
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int n = 1000, m = 200, depth = 4;
    if (argc >= 4) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        depth = atoi(argv[3]);
    }
    
    printf("Running SIMT tests with n=%d, m=%d, depth=%d\n", n, m, depth);
    
    // Allocate and initialize arrays
    size_t total_size = n * m;
    size_t total_size_bytes = total_size * sizeof(int);
    size_t total_float_size = total_size * depth;
    
    int *A = (int *)malloc(total_size_bytes);
    int *B = (int *)malloc(total_size_bytes);
    int *C = (int *)malloc(total_size_bytes);
    int *mask = (int *)malloc(total_size_bytes);
    
    float *X = (float *)malloc(total_float_size * sizeof(float));
    float *Y = (float *)malloc(total_float_size * sizeof(float));
    
    double *D = (double *)malloc(total_size * depth * sizeof(double));
    int *indices = (int *)malloc(total_size * 2 * sizeof(int));
    
    // Initialize with pattern-based data
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        mask[i] = (i % 7 == 0) ? 1 : -1;
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < total_float_size; i++) {
        X[i] = (float)(i % 59) * 0.1f;
        Y[i] = (float)(i % 73) * 0.2f;
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < total_size * depth; i++) {
        D[i] = (double)(i % 101) * 0.01;
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < total_size * 2; i++) {
        indices[i] = (i * 5) % total_size;
    }
    
    // Execute test functions
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m, 42);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, n, m);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, n, m, depth);
    
    printf("Running test_mixed_constructs...\n");
    test_mixed_constructs(A, total_size, 17);
    
    // Compute and print checksums
    unsigned long checksum_A = compute_checksum(A, total_size_bytes);
    unsigned long checksum_X = compute_checksum(X, total_float_size * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, total_size * depth * sizeof(double));
    
    printf("Checksum A: %lu\n", checksum_A);
    printf("Checksum X: %lu\n", checksum_X);
    printf("Checksum D: %lu\n", checksum_D);
    
    // Cleanup
    free(A); free(B); free(C); free(mask);
    free(X); free(Y); free(D); free(indices);
    
    return 0;
}
