#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 5381

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m]) \
        num_teams(n/32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            // Complex conditional that depends on loop indices
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((i ^ j) % 5 == 1) {
                A[idx] = B[idx] - C[idx] * iter;
            } else {
                A[idx] = (B[idx] + C[idx]) / (iter % 7 + 1);
            }
            
            // Additional conditional with thread/team info
            int tid = omp_get_thread_num();
            int team = omp_get_team_num();
            if ((tid % 4) == (team % 4)) {
                A[idx] += (tid << 16) | (team & 0xFFFF);
            }
        }
    }
}

/* Test function 2: Mapped data with pointer-based accesses and SIMD */
void test_simt_mapped(float *X, float *Y, int *perm, int size, float alpha) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, alpha) map(tofrom: X[0:size]) map(to: Y[0:size], perm[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i++) {
        int src_idx = perm[i];
        // Indirect access pattern that requires memory coalescing
        float val = Y[src_idx] * alpha;
        
        // Conditional store with SIMD-friendly pattern
        if (src_idx % 8 < 4) {
            X[i] = val + sinf((float)i * 0.01f);
        } else {
            X[i] = val * cosf((float)i * 0.005f);
        }
        
        // Additional SIMD-width dependent operation
        int lane = i % 32;
        if (lane < 16) {
            X[i] += 1.0f / (lane + 1);
        } else {
            X[i] -= 0.5f / (lane - 15);
        }
    }
}

/* Test function 3: Conditional execution based on thread ID */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp parallel for simd collapse(2) schedule(static)
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r * cols + c;
                int tid = omp_get_thread_num();
                
                // SIMT-relevant condition: different threads take different paths
                if (tid % 2 == 0) {
                    // Even threads: polynomial computation
                    double x = D[idx];
                    D[idx] = x * x * x - 2.0 * x * x + 3.0 * x - 4.0;
                } else {
                    // Odd threads: trigonometric computation
                    D[idx] = sin(D[idx] * 0.1) * cos(D[idx] * 0.05);
                }
                
                // Mask-based conditional with thread-dependent offset
                if (mask[idx] > 0) {
                    int team = omp_get_team_num();
                    D[idx] += (team % 10) * 0.01;
                }
            }
        }
    }
}

/* Test function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int n, int stride) {
    // First: teams distribute
    #pragma omp target teams distribute map(to: n, stride) map(tofrom: data[0:n])
    for (int i = 0; i < n; i += stride) {
        // Nested parallel for simd
        #pragma omp parallel for simd
        for (int j = 0; j < stride && (i + j) < n; j++) {
            int idx = i + j;
            int tid = omp_get_thread_num();
            data[idx] = (data[idx] ^ (tid << (idx % 16))) + (i / stride);
        }
    }
    
    // Second: direct target simd
    #pragma omp target simd map(to: n) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        if (data[i] % 7 == 0) {
            data[i] = data[i] * 3 + 1;
        } else {
            data[i] = data[i] / 2;
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
    int base_size = 1000;
    int iterations = 100;
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running SIMT transformation test with size=%d, iterations=%d\n", 
           base_size, iterations);
    
    // Calculate dimensions for nested loops
    int n = base_size;
    int m = (base_size / 4) + 1;
    int total_elements = n * m;
    
    // Allocate and initialize arrays
    int *A = (int *)malloc(total_elements * sizeof(int));
    int *B = (int *)malloc(total_elements * sizeof(int));
    int *C = (int *)malloc(total_elements * sizeof(int));
    int *mask = (int *)malloc(total_elements * sizeof(int));
    
    float *X = (float *)malloc(total_elements * sizeof(float));
    float *Y = (float *)malloc(total_elements * sizeof(float));
    int *perm = (int *)malloc(total_elements * sizeof(int));
    
    double *D = (double *)malloc(total_elements * sizeof(double));
    
    // Initialize with pattern-based data
    for (int i = 0; i < total_elements; i++) {
        A[i] = i % 97;
        B[i] = (i * 3) % 113;
        C[i] = (i * 5) % 131;
        mask[i] = (i % 11) > 5 ? 1 : 0;
        
        X[i] = (float)(i % 101) * 0.1f;
        Y[i] = (float)((i * 7) % 103) * 0.05f;
        perm[i] = (i * 11) % total_elements;
        
        D[i] = (double)(i % 107) * 0.01;
    }
    
    // Execute test functions multiple times with different parameters
    for (int iter = 0; iter < iterations; iter++) {
        // Vary parameters to explore different code paths
        int current_n = n - (iter % 7);
        int current_m = m + (iter % 5);
        
        test_simt_nested(A, B, C, current_n, current_m, iter);
        
        if (iter % 3 == 0) {
            float alpha = 1.0f + (iter * 0.01f);
            test_simt_mapped(X, Y, perm, total_elements / 2, alpha);
        }
        
        if (iter % 4 == 0) {
            int rows = 32 + (iter % 16);
            int cols = 32 + (iter % 8);
            test_simt_conditional(D, mask, rows, cols);
        }
        
        if (iter % 5 == 0) {
            int stride = 16 + (iter % 32);
            test_mixed_constructs(A, total_elements / 4, stride);
        }
    }
    
    // Compute and print checksums
    unsigned long checksum_A = compute_checksum(A, total_elements * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, total_elements * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, total_elements * sizeof(double));
    
    printf("Checksums:\n");
    printf("  Array A: %lu\n", checksum_A);
    printf("  Array X: %lu\n", checksum_X);
    printf("  Array D: %lu\n", checksum_D);
    
    // Cleanup
    free(A); free(B); free(C); free(mask);
    free(X); free(Y); free(perm);
    free(D);
    
    return 0;
}
