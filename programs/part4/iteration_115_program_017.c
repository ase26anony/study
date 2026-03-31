#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 1000000007

// Test 1: Nested loops with collapse and conditional inside SIMD
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            // Conditional execution inside SIMD loop - may trigger SIMT transformation
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] / 2 + C[idx];
            } else {
                A[idx] = B[idx] + C[idx] * 3;
            }
            
            // Additional conditional based on iteration count
            if (iter > 100 && j % 4 == 0) {
                A[idx] += (i % 5) * (j % 7);
            }
        }
    }
}

// Test 2: Complex pointer-based accesses with SIMD safelen
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simd safelen(16)
    for (int i = 0; i < size; i += stride) {
        // Complex pointer arithmetic and indirect access
        int idx = indices[i % size];
        float *ptr = &Y[idx];
        
        // Multiple conditional paths
        if (i % 8 == 0) {
            X[i] = *ptr * 2.0f + sinf((float)i * 0.1f);
        } else if (i % 8 == 4) {
            X[i] = *ptr * 0.5f + cosf((float)i * 0.1f);
        } else {
            X[i] = *ptr + (float)(i % 16) * 0.25f;
        }
        
        // Nested condition with pointer dereference
        if (ptr != NULL && *ptr > 0.0f) {
            X[i] = fabsf(X[i]) + 1.0f;
        }
    }
}

// Test 3: Conditional execution based on thread ID
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            // Thread-dependent conditional - may force SIMT transformation
            int thread_id = omp_get_thread_num();
            
            if (thread_id % 2 == 0) {
                // Even threads use one computation path
                D[idx] = (double)(i * cols + j) * 1.5;
                if (mask[idx] > 0) {
                    D[idx] += sin((double)thread_id * 0.01);
                }
            } else {
                // Odd threads use different computation path
                D[idx] = (double)(i + j) * 2.5;
                if (mask[idx] < 0) {
                    D[idx] -= cos((double)thread_id * 0.01);
                }
            }
            
            // Additional SIMD lane-dependent condition
            if ((j % 4) == (thread_id % 4)) {
                D[idx] *= 1.1;
            }
        }
    }
}

// Test 4: Mixed constructs - target region with nested parallel for simd
void test_mixed_constructs(int *out, const int *in1, const int *in2, int n) {
    #pragma omp target map(to: n, in1[0:n], in2[0:n]) map(tofrom: out[0:n])
    {
        #pragma omp parallel for simd
        for (int i = 0; i < n; i++) {
            // Complex conditional with multiple branches
            int val1 = in1[i];
            int val2 = in2[i];
            
            if (val1 > val2) {
                out[i] = val1 - val2;
                if (out[i] > 100) {
                    out[i] = 100;
                }
            } else if (val1 < val2) {
                out[i] = val2 - val1;
                if (out[i] < 0) {
                    out[i] = 0;
                }
            } else {
                out[i] = val1 * val2 % 97;
                if (i % 3 == 0) {
                    out[i] += omp_get_thread_num();
                }
            }
        }
    }
}

// Test 5: Multiple SIMD clauses with different parameters
void test_multiple_simd(float *result, const float *data, int dim1, int dim2, int dim3) {
    #pragma omp target teams distribute parallel for simd collapse(3) \
        map(to: dim1, dim2, dim3, data[0:dim1*dim2*dim3]) \
        map(tofrom: result[0:dim1*dim2*dim3]) \
        simd linear(i:1) aligned(result, data:64)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                int idx = (i * dim2 + j) * dim3 + k;
                
                // Multiple conditionals that depend on all loop indices
                if ((i + j + k) % 5 == 0) {
                    result[idx] = data[idx] * 2.0f;
                } else if ((i * j * k) % 7 == 0) {
                    result[idx] = data[idx] / 2.0f;
                } else {
                    result[idx] = data[idx] + (float)((i ^ j ^ k) & 0xFF);
                }
                
                // Additional SIMD lane-specific operation
                if (k % 8 == omp_get_thread_num() % 8) {
                    result[idx] = fmaxf(result[idx], 0.0f);
                }
            }
        }
    }
}

// Compute checksum to verify execution
unsigned long long compute_checksum(int *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + (unsigned long long)arr[i]) % CHECKSUM_MOD;
    }
    return sum;
}

unsigned long long compute_float_checksum(float *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        // Convert float to integer representation for checksum
        unsigned int val;
        memcpy(&val, &arr[i], sizeof(float));
        sum = (sum * 31 + (unsigned long long)val) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments for sizes
    int base_size = 256;
    int iterations = 10;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running SIMT transformation tests with size=%d, iterations=%d\n", 
           base_size, iterations);
    
    // Dynamically allocate arrays with varying sizes
    int n = base_size;
    int m = base_size / 2;
    int total_int = n * m;
    int total_float = base_size * 2;
    int total_double = (base_size / 4) * (base_size / 4);
    
    int *A = (int*)malloc(total_int * sizeof(int));
    int *B = (int*)malloc(total_int * sizeof(int));
    int *C = (int*)malloc(total_int * sizeof(int));
    int *indices = (int*)malloc(total_float * sizeof(int));
    int *mask = (int*)malloc(total_double * sizeof(int));
    int *out = (int*)malloc(base_size * sizeof(int));
    int *in1 = (int*)malloc(base_size * sizeof(int));
    int *in2 = (int*)malloc(base_size * sizeof(int));
    
    float *X = (float*)malloc(total_float * sizeof(float));
    float *Y = (float*)malloc(total_float * sizeof(float));
    float *result = (float*)malloc(base_size * base_size * base_size / 8 * sizeof(float));
    float *data = (float*)malloc(base_size * base_size * base_size / 8 * sizeof(float));
    
    double *D = (double*)malloc(total_double * sizeof(double));
    
    // Initialize arrays with pattern-based data
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        C[i] = (i * 7) % 101;
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 127) * 0.1f;
        indices[i] = (i * 5) % total_float;
    }
    
    for (int i = 0; i < total_double; i++) {
        D[i] = 0.0;
        mask[i] = (i % 3) - 1;  // Values: -1, 0, 1
    }
    
    for (int i = 0; i < base_size; i++) {
        out[i] = 0;
        in1[i] = (i * 11) % 89;
        in2[i] = (i * 13) % 79;
    }
    
    int data_size = base_size * base_size * base_size / 8;
    for (int i = 0; i < data_size; i++) {
        data[i] = (float)(i % 255) * 0.01f;
        result[i] = 0.0f;
    }
    
    // Execute test functions multiple times with different parameters
    unsigned long long total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        // Test 1: Nested loops with collapse
        test_simt_nested(A, B, C, n, m, iter);
        total_checksum = (total_checksum + compute_checksum(A, total_int)) % CHECKSUM_MOD;
        
        // Test 2: Pointer-based accesses with SIMD safelen
        test_simt_mapped(X, Y, indices, total_float, 1 + iter % 4);
        total_checksum = (total_checksum + compute_float_checksum(X, total_float)) % CHECKSUM_MOD;
        
        // Test 3: Thread-dependent conditionals
        test_simt_conditional(D, mask, base_size / 4, base_size / 4);
        
        // Test 4: Mixed constructs
        test_mixed_constructs(out, in1, in2, base_size);
        total_checksum = (total_checksum + compute_checksum(out, base_size)) % CHECKSUM_MOD;
        
        // Test 5: Multiple SIMD clauses
        test_multiple_simd(result, data, base_size / 2, base_size / 2, base_size / 4);
        total_checksum = (total_checksum + compute_float_checksum(result, data_size)) % CHECKSUM_MOD;
        
        // Modify some inputs for next iteration
        if (iter < iterations - 1) {
            for (int i = 0; i < total_int; i++) {
                B[i] = (B[i] + 1) % 97;
                C[i] = (C[i] + 2) % 101;
            }
        }
    }
    
    printf("Final checksum: %llu\n", total_checksum);
    
    // Cleanup
    free(A); free(B); free(C);
    free(X); free(Y); free(indices);
    free(D); free(mask);
    free(out); free(in1); free(in2);
    free(result); free(data);
    
    return 0;
}
