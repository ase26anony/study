#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

// Function 1: Nested loops with SIMD clause and conditional inside
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            // Conditional execution path inside SIMD loop
            // This should trigger the conditional structure with labels
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] - C[idx];
            } else {
                A[idx] = B[idx] + C[idx] * 3;
            }
            
            // Additional control flow with thread index
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

// Function 2: Complex pointer-based accesses with SIMD
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, indices[0:size], Y[0:size*stride]) \
        map(tofrom: X[0:size]) \
        safelen(16)
    for (int i = 0; i < size; i++) {
        // Indirect memory access pattern - may influence SIMT transformation
        int idx = indices[i] % size;
        
        // Complex computation with conditionals
        float temp = Y[i * stride + idx];
        if (temp > 0.5f) {
            X[i] = sinf(temp) * cosf(Y[idx * stride + i]);
        } else {
            X[i] = sqrtf(fabsf(temp)) + Y[i * stride] * 0.5f;
        }
        
        // Nested condition based on SIMD lane pattern
        if ((i & 0xF) == 0) {  // Check SIMD lane
            X[i] *= 2.0f;
        }
    }
}

// Function 3: Conditional execution with thread-dependent paths
void test_simt_conditional(int *data, int *mask, int rows, int cols) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: rows, cols, mask[0:rows*cols]) map(tofrom: data[0:rows*cols])
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            // Thread-dependent conditional - may trigger SIMT transformation
            int thread_mod = omp_get_thread_num() % 8;
            
            switch (thread_mod) {
                case 0:
                case 1:
                    data[idx] = mask[idx] * 2;
                    break;
                case 2:
                case 3:
                    data[idx] = mask[idx] + thread_mod;
                    break;
                case 4:
                case 5:
                    data[idx] = mask[idx] - (i * cols + j);
                    break;
                default:
                    data[idx] = mask[idx] ^ (i * cols + j);
                    break;
            }
            
            // Additional conditional based on loop indices
            if (i > rows/2 && j < cols/2) {
                data[idx] += 1000;
            }
        }
    }
}

// Function 4: Mixed constructs - target with nested parallel for simd
void test_mixed_constructs(double *matrix, int dim, int iter) {
    #pragma omp target map(tofrom: matrix[0:dim*dim]) map(to: dim, iter)
    {
        #pragma omp teams distribute
        for (int team = 0; team < dim; team++) {
            #pragma omp parallel for simd
            for (int i = 0; i < dim; i++) {
                int idx = team * dim + i;
                
                // Complex conditional that depends on multiple factors
                if ((team + i + iter) % 5 == 0) {
                    matrix[idx] = sin(matrix[idx]) * cos(matrix[i * dim + team]);
                } else if (omp_get_num_threads() > 4) {
                    matrix[idx] = matrix[idx] * 0.8 + matrix[(i + team) % dim] * 0.2;
                } else {
                    matrix[idx] = sqrt(fabs(matrix[idx]));
                }
            }
        }
    }
}

// Helper function to compute checksum
unsigned long long compute_checksum(int *data, int size) {
    unsigned long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum = (checksum * 31 + (data[i] % CHECKSUM_MOD)) % 1000000007;
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments for sizes
    int base_size = 256;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    // Dynamically allocate arrays with varying sizes
    int n = base_size;
    int m = base_size / 2;
    int total_int = n * m;
    int total_float = base_size * 16;  // For stride access
    
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *mask = (int *)malloc(total_int * sizeof(int));
    
    float *X = (float *)malloc(total_float * sizeof(float));
    float *Y = (float *)malloc(total_float * sizeof(float));
    int *indices = (int *)malloc(base_size * sizeof(int));
    
    double *matrix = (double *)malloc(base_size * base_size * sizeof(double));
    
    // Initialize arrays with pattern-based data
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        mask[i] = (i % 7) * ((i % 13) + 1);
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = 0.0f;
        Y[i] = (float)((i * 7) % 131) / 131.0f;
    }
    
    for (int i = 0; i < base_size; i++) {
        indices[i] = (i * 11) % base_size;
    }
    
    for (int i = 0; i < base_size * base_size; i++) {
        matrix[i] = (double)((i * 5) % 151) / 151.0;
    }
    
    // Execute test functions multiple times with different parameters
    for (int iter = 0; iter < iterations; iter++) {
        // Vary parameters to hit different code paths
        int current_n = n + (iter % 5);
        int current_m = m + (iter % 3);
        
        test_simt_nested(A, B, C, current_n, current_m);
        
        if (iter % 2 == 0) {
            test_simt_mapped(X, Y, indices, base_size, 16);
        }
        
        test_simt_conditional(mask, C, current_n, current_m);
        
        if (iter % 3 == 0) {
            test_mixed_constructs(matrix, base_size, iter);
        }
    }
    
    // Compute checksums to ensure all code paths executed
    unsigned long long checksum_A = compute_checksum(A, total_int);
    unsigned long long checksum_mask = compute_checksum(mask, total_int);
    
    printf("Checksum A: %llu\n", checksum_A);
    printf("Checksum mask: %llu\n", checksum_mask);
    
    // Verify some values
    int verify_count = 0;
    for (int i = 0; i < 10 && i < total_int; i++) {
        if (A[i] != 0) verify_count++;
    }
    printf("Non-zero values in first 10 elements of A: %d\n", verify_count);
    
    // Cleanup
    free(A);
    free(B);
    free(C);
    free(mask);
    free(X);
    free(Y);
    free(indices);
    free(matrix);
    
    return 0;
}
