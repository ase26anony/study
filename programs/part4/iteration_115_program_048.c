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
            if ((i * j) % 7 == 0) {
                A[i * m + j] = B[i] + C[j];
            } else if ((i + j) % 3 == 0) {
                A[i * m + j] = B[i] * C[j];
            } else {
                // Nested condition with thread ID dependency
                int tid = omp_get_thread_num();
                if (tid % 4 == 0) {
                    A[i * m + j] = B[i] - C[j];
                } else {
                    A[i * m + j] = B[i] / (C[j] + 1);
                }
            }
            
            // Additional computation to prevent dead code elimination
            A[i * m + j] += (i & 0xF) | (j & 0xF);
        }
    }
}

// Test 2: Pointer-based indirect accesses with SIMD
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size*stride]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i++) {
        int idx = indices[i];
        // Indirect access pattern - harder to optimize
        float val = Y[idx % size];
        
        // SIMD-friendly but with conditional store
        if (val > 0.5f) {
            X[i * stride] = val * 2.0f;
        } else {
            X[i * stride] = val / 2.0f;
        }
        
        // Additional pointer arithmetic
        float *ptr = &X[i * stride];
        for (int k = 0; k < 3; k++) {
            ptr[k] += (float)(i % (k + 2)) * 0.1f;
        }
    }
}

// Test 3: Multiple nested parallel regions with SIMD
void test_simt_conditional(double *D, int *mask, int rows, int cols, int depth) {
    #pragma omp target teams distribute parallel for collapse(2) \
        map(tofrom: D[0:rows*cols*depth]) map(to: mask[0:rows*cols]) \
        num_teams(rows*cols/64)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            // Inner SIMD loop with thread-dependent condition
            #pragma omp simd reduction(+:D[idx*depth]) \
                linear(k:1) aligned(D:64)
            for (int k = 0; k < depth; k++) {
                int tid = omp_get_thread_num();
                int lane = tid % 32;  // SIMD lane approximation
                
                if (mask[idx] > 0) {
                    if (lane % 2 == 0) {
                        D[idx * depth + k] += sin((double)k * 0.1) * lane;
                    } else {
                        D[idx * depth + k] += cos((double)k * 0.1) * (lane + 1);
                    }
                } else {
                    D[idx * depth + k] = (double)((i + j + k) % 256);
                }
                
                // Additional branching based on computed value
                if (D[idx * depth + k] > 128.0) {
                    D[idx * depth + k] = sqrt(D[idx * depth + k]);
                }
            }
        }
    }
}

// Test 4: Mixed constructs - target with nested parallel for simd
void test_mixed_constructs(int *out, const int *in1, const int *in2, 
                          int dim1, int dim2, int dim3) {
    #pragma omp target map(to: in1[0:dim1*dim2], in2[0:dim2*dim3]) \
                       map(tofrom: out[0:dim1*dim3]) \
        device(0) nowait
    {
        #pragma omp teams distribute parallel for collapse(2) \
            num_teams(dim1*dim3/128) thread_limit(128)
        for (int i = 0; i < dim1; i++) {
            for (int k = 0; k < dim3; k++) {
                int sum = 0;
                
                // Innermost SIMD loop with reduction
                #pragma omp simd reduction(+:sum) safelen(16)
                for (int j = 0; j < dim2; j++) {
                    int tid = omp_get_thread_num();
                    int val = in1[i * dim2 + j] * in2[j * dim3 + k];
                    
                    // Thread/SIMD lane dependent operation
                    if ((tid + i + j + k) % 8 < 4) {
                        sum += val;
                    } else {
                        sum -= val;
                    }
                }
                
                out[i * dim3 + k] = sum;
            }
        }
    }
    #pragma omp taskwait
}

// Helper function to compute checksum
unsigned long long compute_checksum(int *data, int size) {
    unsigned long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum = (checksum * 31 + (unsigned long long)data[i]) % CHECKSUM_MOD;
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments for flexibility
    int n = 512, m = 256, p = 128;
    if (argc >= 4) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        p = atoi(argv[3]);
    }
    
    printf("Running SIMT transformation tests with n=%d, m=%d, p=%d\n", n, m, p);
    
    // Dynamic allocation with different patterns
    int *A = (int*)malloc(n * m * sizeof(int));
    int *B = (int*)malloc(n * sizeof(int));
    int *C = (int*)malloc(m * sizeof(int));
    int *indices = (int*)malloc(p * sizeof(int));
    float *X = (float*)malloc(p * 4 * sizeof(float));
    float *Y = (float*)malloc(p * sizeof(float));
    double *D = (double*)malloc(n * m * 8 * sizeof(double));
    int *mask = (int*)malloc(n * m * sizeof(int));
    int *out = (int*)malloc(n * p * sizeof(int));
    int *in1 = (int*)malloc(n * m * sizeof(int));
    int *in2 = (int*)malloc(m * p * sizeof(int));
    
    // Initialize with pattern-based data
    for (int i = 0; i < n; i++) {
        B[i] = i % 97;
        for (int j = 0; j < m; j++) {
            A[i * m + j] = (i * j) % 256;
            mask[i * m + j] = ((i + j) % 3 == 0) ? 1 : 0;
            for (int k = 0; k < 8; k++) {
                D[(i * m + j) * 8 + k] = (double)((i + j + k) % 1000);
            }
        }
    }
    
    for (int i = 0; i < m; i++) {
        C[i] = (i * 7) % 89;
    }
    
    for (int i = 0; i < p; i++) {
        indices[i] = (i * 13) % p;
        Y[i] = (float)(i % 100) / 100.0f;
        for (int j = 0; j < 4; j++) {
            X[i * 4 + j] = (float)((i + j) % 50);
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            in1[i * m + j] = (i + j * 3) % 127;
        }
    }
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            in2[i * p + j] = (i * 5 + j * 7) % 131;
        }
    }
    
    // Execute test functions with different OpenMP constructs
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m, 1);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, p, 4);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, n, m, 8);
    
    printf("Running test_mixed_constructs...\n");
    test_mixed_constructs(out, in1, in2, n, m, p);
    
    // Compute and print checksums to ensure execution
    unsigned long long checksum1 = compute_checksum(A, n * m);
    unsigned long long checksum2 = compute_checksum((int*)X, p * 4);
    unsigned long long checksum3 = compute_checksum((int*)D, n * m * 8 * 2);
    unsigned long long checksum4 = compute_checksum(out, n * p);
    
    printf("Checksums:\n");
    printf("  test_simt_nested: %llu\n", checksum1);
    printf("  test_simt_mapped: %llu\n", checksum2);
    printf("  test_simt_conditional: %llu\n", checksum3);
    printf("  test_mixed_constructs: %llu\n", checksum4);
    
    // Cleanup
    free(A); free(B); free(C); free(indices);
    free(X); free(Y); free(D); free(mask);
    free(out); free(in1); free(in2);
    
    return 0;
}
