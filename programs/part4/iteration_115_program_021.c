#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n], C[0:m]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Complex conditional that depends on both loop indices
            // This should create control flow within SIMD lanes
            if ((i + j) % 3 == 0) {
                A[idx] = B[i] * C[j] + iter;
            } else if ((i ^ j) & 1) {
                A[idx] = B[i] - C[j] + idx;
            } else {
                A[idx] = (B[i] + C[j]) * 2 - idx;
            }
            
            // Additional conditional with thread ID dependency
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += tid;
            }
        }
    }
}

/* Test 2: Pointer-based indirect accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size*2], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        // Indirect access pattern - encourages memory coalescing analysis
        int idx = indices[i % size];
        float temp = Y[idx * 2] + Y[idx * 2 + 1];
        
        // Conditional store with SIMD divergence
        if (temp > 0.5f) {
            X[i] = temp * sinf((float)i * 0.1f);
        } else {
            X[i] = temp * cosf((float)i * 0.1f);
        }
        
        // Additional pointer arithmetic
        float *ptr = &X[i];
        if (i % 8 == 0) {
            *ptr *= 2.0f;
        }
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int offset) {
    #pragma omp target map(tofrom: D[0:rows*cols]) map(to: mask[0:rows]) \
        num_teams(8)
    {
        #pragma omp parallel
        {
            int team_id = omp_get_team_num();
            int thread_id = omp_get_thread_num();
            
            #pragma omp for simd collapse(2) nowait
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    int idx = r * cols + c;
                    
                    // Complex conditional based on thread/team IDs
                    // This should generate the cond variable and labels
                    if ((team_id + thread_id) % 2 == 0) {
                        if (mask[r] > 0) {
                            D[idx] = (double)(r * c) / (1.0 + D[idx]) + offset;
                        } else {
                            D[idx] = (double)(r + c) * sqrt(D[idx] + 1.0);
                        }
                    } else {
                        D[idx] = (double)(thread_id * team_id) / (idx + 1.0);
                    }
                    
                    // Additional SIMD lane divergence
                    if (thread_id % 3 == 0) {
                        D[idx] += sin(D[idx] * 0.01);
                    }
                }
            }
        }
    }
}

/* Test 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *out, const int *in, int n, int scale) {
    // First a simple target region
    #pragma omp target teams distribute parallel for \
        map(tofrom: out[0:n]) map(to: in[0:n])
    for (int i = 0; i < n; i++) {
        out[i] = in[i] * scale;
    }
    
    // Then a SIMD-only region
    #pragma omp target teams distribute simd \
        map(tofrom: out[0:n]) simdlen(8)
    for (int i = 0; i < n; i++) {
        if (out[i] % 7 == 0) {
            out[i] = out[i] / 2;
        }
    }
}

/* Helper to compute checksum */
unsigned long compute_checksum(void *data, size_t size_bytes) {
    unsigned long checksum = 0;
    unsigned char *bytes = (unsigned char *)data;
    for (size_t i = 0; i < size_bytes; i++) {
        checksum = (checksum * 31 + bytes[i]) % CHECKSUM_MOD;
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int base_size = 1000;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    // Calculate dimensions with non-constant bounds
    int n = base_size + (iterations % 17);  // Non-constant
    int m = base_size / 2 + (iterations % 13);  // Non-constant
    int total_int = n * m;
    int total_float = base_size * 2;
    
    // Dynamic allocations
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(n * sizeof(int));
    int *C = (int *)malloc(m * sizeof(int));
    float *X = (float *)malloc(total_float * sizeof(float));
    float *Y = (float *)malloc(total_float * 2 * sizeof(float));
    double *D = (double *)malloc(n * m * sizeof(double));
    int *indices = (int *)malloc(base_size * sizeof(int));
    int *mask = (int *)malloc(n * sizeof(int));
    
    // Initialize with pattern-based data
    for (int i = 0; i < total_int; i++) {
        A[i] = i % 97;
    }
    for (int i = 0; i < n; i++) {
        B[i] = (i * 3) % 113;
        mask[i] = (i % 5 == 0) ? 1 : 0;
    }
    for (int i = 0; i < m; i++) {
        C[i] = (i * 7) % 89;
    }
    for (int i = 0; i < total_float; i++) {
        X[i] = (float)(i % 71) * 0.1f;
        Y[i] = (float)(i % 59) * 0.2f;
    }
    for (int i = 0; i < base_size; i++) {
        indices[i] = (i * 11) % base_size;
    }
    for (int i = 0; i < n * m; i++) {
        D[i] = (double)(i % 43) * 0.01;
    }
    
    // Execute test functions multiple times with varying parameters
    for (int iter = 0; iter < iterations; iter++) {
        // Vary parameters to prevent constant propagation
        int current_n = n + (iter % 3);
        int current_m = m - (iter % 2);
        
        test_simt_nested(A, B, C, current_n, current_m, iter);
        
        int stride = 1 + (iter % 4);
        test_simt_mapped(X, Y, indices, base_size, stride);
        
        test_simt_conditional(D, mask, current_n, current_m, iter);
        
        // Test mixed constructs every few iterations
        if (iter % 7 == 0) {
            test_mixed_constructs(B, C, current_m, iter + 1);
        }
    }
    
    // Compute checksums of final states
    unsigned long checksum_A = compute_checksum(A, total_int * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, total_float * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, n * m * sizeof(double));
    
    printf("Checksums: A=%lu, X=%lu, D=%lu\n", 
           checksum_A, checksum_X, checksum_D);
    
    // Cleanup
    free(A); free(B); free(C);
    free(X); free(Y); free(D);
    free(indices); free(mask);
    
    return 0;
}
