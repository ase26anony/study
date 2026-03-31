#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(n/32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional that depends on thread index and loop variables */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((i * j) % 5 == 0) {
                A[idx] = B[idx] - C[idx] - iter;
            } else {
                A[idx] = (B[idx] + C[idx]) * iter;
            }
            
            /* Additional conditional with thread-specific behavior */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += tid;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Indirect access pattern that requires memory coalescing */
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            /* Complex conditional with floating point operations */
            if (Y[idx] > 0.5f) {
                X[i] = Y[idx] * sinf((float)i * 0.01f);
            } else if (Y[idx] < -0.5f) {
                X[i] = Y[idx] * cosf((float)i * 0.01f);
            } else {
                X[i] = Y[idx] * tanf((float)i * 0.01f);
            }
            
            /* SIMD-friendly but with control flow */
            for (int k = 0; k < 4; k++) {
                if ((i + k) % 8 == 0) {
                    X[i] += k * 0.1f;
                }
            }
        }
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp teams distribute
        for (int i = 0; i < rows; i++) {
            #pragma omp parallel for simd
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Thread-dependent conditional */
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((tid + team_id) % 2 == 0) {
                    D[idx] = (mask[idx] > 0) ? 
                             D[idx] * exp(-(double)idx * 0.001) :
                             D[idx] * exp((double)idx * 0.001);
                } else {
                    D[idx] = (mask[idx] > 0) ?
                             D[idx] / (1.0 + (double)idx * 0.001) :
                             D[idx] * (1.0 + (double)idx * 0.001);
                }
                
                /* Nested conditional inside SIMD loop */
                if (j % 16 == 0) {
                    D[idx] += (double)(tid % 8) * 0.01;
                }
            }
        }
    }
}

/* Test 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *out, int *in1, int *in2, int dim1, int dim2, int dim3) {
    /* First target region with distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: dim1, dim2, dim3) map(tofrom: out[0:dim1*dim2*dim3]) \
        map(to: in1[0:dim1*dim2*dim3], in2[0:dim1*dim2*dim3])
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                int idx = (i * dim2 + j) * dim3 + k;
                out[idx] = in1[idx] ^ in2[idx];
                
                /* Complex conditional chain */
                if (i % 3 == 0 && j % 4 == 0) {
                    out[idx] += k;
                } else if (i % 5 == 0 || j % 6 == 0) {
                    out[idx] -= k;
                }
                
                if (omp_get_thread_num() % 3 == 0) {
                    out[idx] *= 2;
                }
            }
        }
    }
    
    /* Second target region with different structure */
    #pragma omp target teams distribute parallel for \
        map(to: dim1, dim2) map(tofrom: out[0:dim1*dim2]) \
        num_teams(dim1/16) thread_limit(128)
    for (int i = 0; i < dim1; i++) {
        #pragma omp simd
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            if ((i + j) % 7 == 0) {
                out[idx] = out[idx] % 256;
            }
        }
    }
}

/* Helper function to compute checksum */
unsigned long long compute_checksum(void *data, size_t size_bytes) {
    unsigned long long checksum = 0;
    unsigned char *bytes = (unsigned char *)data;
    for (size_t i = 0; i < size_bytes; i++) {
        checksum = (checksum * 31 + bytes[i]) % CHECKSUM_MOD;
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <size_n> <size_m>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int total_size = n * m;
    
    if (n <= 0 || m <= 0) {
        printf("Error: sizes must be positive\n");
        return 1;
    }
    
    /* Allocate and initialize arrays with pattern-based data */
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    double *D = (double *)malloc(total_size * sizeof(double));
    int *indices = (int *)malloc(total_size * sizeof(int));
    
    if (!A || !B || !C || !mask || !X || !Y || !D || !indices) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        mask[i] = (i % 13 == 0) ? 1 : -1;
        X[i] = sinf((float)i * 0.1f);
        Y[i] = cosf((float)i * 0.05f);
        D[i] = (double)i * 0.01;
        indices[i] = (i * 7) % total_size;
    }
    
    printf("Initialized arrays of size %d x %d = %d elements\n", n, m, total_size);
    
    /* Execute test functions with different OpenMP constructs */
    test_simt_nested(A, B, C, n, m, 42);
    printf("Completed test_simt_nested\n");
    
    test_simt_mapped(X, Y, indices, total_size, 2);
    printf("Completed test_simt_mapped\n");
    
    test_simt_conditional(D, mask, n, m);
    printf("Completed test_simt_conditional\n");
    
    test_mixed_constructs(A, B, C, n/2, m, 2);
    printf("Completed test_mixed_constructs\n");
    
    /* Compute and print checksums to verify execution */
    unsigned long long checksum_A = compute_checksum(A, total_size * sizeof(int));
    unsigned long long checksum_X = compute_checksum(X, total_size * sizeof(float));
    unsigned long long checksum_D = compute_checksum(D, total_size * sizeof(double));
    
    printf("\nFinal checksums:\n");
    printf("Array A (int): %llu\n", checksum_A);
    printf("Array X (float): %llu\n", checksum_X);
    printf("Array D (double): %llu\n", checksum_D);
    
    /* Cleanup */
    free(A);
    free(B);
    free(C);
    free(mask);
    free(X);
    free(Y);
    free(D);
    free(indices);
    
    return 0;
}
