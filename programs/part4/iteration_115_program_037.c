#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <assert.h>

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            /* This should trigger the conditional structure with lab1, lab2, lab3 */
            if ((i + j) % 3 == 0) {
                /* Complex memory access pattern */
                A[idx] = B[idx] * C[idx] + i - j;
            } else if ((i * j) % 5 == 0) {
                /* Another conditional path */
                A[idx] = B[C[idx] % (n*m)] + j;
            } else {
                /* Default path */
                A[idx] = B[idx] + C[idx];
            }
            
            /* Additional conditional that might affect SIMT transformation */
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test function 2: Mapped pointers with indirect accesses and SIMD clause */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: Y[0:size], indices[0:size]) map(tofrom: X[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access - important for SIMT transformation */
        int idx = indices[i] % size;
        
        /* Complex computation with conditional */
        if (idx % 2 == 0) {
            X[i] = Y[idx] * 2.0f + (float)(i % 8);
        } else {
            X[i] = Y[idx] / 2.0f - (float)(i % 8);
        }
        
        /* Nested conditional based on thread index */
        if (omp_get_thread_num() % 3 == 0) {
            X[i] += 0.5f;
        }
    }
}

/* Test function 3: Target region with nested parallel for simd */
void test_simt_conditional(double *D, int *mask, int nrows, int ncols) {
    #pragma omp target map(to: mask[0:nrows*ncols]) map(tofrom: D[0:nrows*ncols]) \
        num_teams(8)
    {
        #pragma omp parallel for simd collapse(2) \
            simdlen(8) aligned(D:64) if(nrows > 100)
        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols; j++) {
                int idx = i * ncols + j;
                
                /* Conditional that depends on thread index - may trigger SIMT */
                if (omp_get_thread_num() % 2 == 0) {
                    if (mask[idx] > 0) {
                        D[idx] = D[idx] * 1.5 + (double)(i + j);
                    } else {
                        D[idx] = D[idx] * 0.5 - (double)(i + j);
                    }
                } else {
                    D[idx] = D[idx] + (double)(mask[idx] % 10);
                }
            }
        }
    }
}

/* Test function 4: Mixed directives to explore different code paths */
void test_mixed_simt(int *data, int *temp, int dim1, int dim2, int dim3) {
    #pragma omp target teams distribute parallel for simd collapse(3) \
        map(tofrom: data[0:dim1*dim2*dim3]) map(to: temp[0:dim1*dim2]) \
        num_teams(dim1) thread_limit(256)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                int idx = (i * dim2 + j) * dim3 + k;
                int temp_idx = i * dim2 + j;
                
                /* Complex conditional structure */
                switch ((i + j + k) % 4) {
                    case 0:
                        data[idx] = temp[temp_idx] + k * 2;
                        break;
                    case 1:
                        data[idx] = temp[temp_idx] - k * 3;
                        if (omp_get_thread_num() % 2 == 0) {
                            data[idx] += 10;
                        }
                        break;
                    case 2:
                        data[idx] = temp[temp_idx] * (k + 1);
                        break;
                    default:
                        data[idx] = temp[temp_idx] / ((k % 5) + 1);
                        break;
                }
            }
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *C, int *indices, 
                 float *X, float *Y, 
                 double *D, int *mask,
                 int total_size, int n, int m) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = (i % 97) + 1;
        C[i] = (i * 3) % 113;
        indices[i] = (i * 7) % total_size;
        X[i] = (float)(i % 50) * 0.1f;
        Y[i] = (float)(i % 70) * 0.2f;
        D[i] = (double)(i % 30) * 0.3;
        mask[i] = (i % 3 == 0) ? 1 : -1;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int size) {
    long long sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < size; i++) {
        sum += (long long)A[i];
        sum += (long long)(X[i] * 100);
        sum += (long long)(D[i] * 100);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int n = 256;
    int m = 128;
    int iterations = 10;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    printf("Running with n=%d, m=%d, iterations=%d\n", n, m, iterations);
    
    int total_size = n * m;
    int total_size_3d = n * m * (m/2);
    
    /* Dynamic allocation */
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    int *C = (int*)malloc(total_size * sizeof(int));
    int *indices = (int*)malloc(total_size * sizeof(int));
    int *mask = (int*)malloc(total_size * sizeof(int));
    int *data_3d = (int*)malloc(total_size_3d * sizeof(int));
    int *temp_3d = (int*)malloc(n * m * sizeof(int));
    
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size * sizeof(float));
    
    double *D = (double*)malloc(total_size * sizeof(double));
    
    /* Check allocations */
    assert(A && B && C && indices && mask && data_3d && temp_3d && X && Y && D);
    
    /* Initialize arrays */
    init_arrays(A, B, C, indices, X, Y, D, mask, total_size, n, m);
    
    /* Initialize 3D arrays */
    #pragma omp parallel for
    for (int i = 0; i < total_size_3d; i++) {
        data_3d[i] = i % 100;
    }
    for (int i = 0; i < n * m; i++) {
        temp_3d[i] = (i * 11) % 200;
    }
    
    long long total_checksum = 0;
    
    /* Execute test functions multiple times */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        /* Call all test functions */
        test_simt_nested(A, B, C, n, m);
        test_simt_mapped(X, Y, indices, total_size, 2);
        test_simt_conditional(D, mask, n, m);
        test_mixed_simt(data_3d, temp_3d, n/2, m, m/2);
        
        /* Compute checksum for this iteration */
        long long iter_checksum = compute_checksum(A, X, D, total_size);
        total_checksum += iter_checksum;
        
        printf("  Iteration checksum: %lld\n", iter_checksum);
        
        /* Modify some inputs for next iteration */
        #pragma omp parallel for
        for (int i = 0; i < total_size; i++) {
            B[i] = (B[i] + 1) % 1000;
            indices[i] = (indices[i] + 5) % total_size;
        }
    }
    
    printf("\nTotal checksum across all iterations: %lld\n", total_checksum);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices); free(mask);
    free(data_3d); free(temp_3d);
    free(X); free(Y);
    free(D);
    
    return 0;
}
