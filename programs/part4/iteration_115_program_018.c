#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(16) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            int thread_id = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            if ((thread_id + team_id) % 3 == 0) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((i + j) % 5 == 0) {
                A[idx] = B[idx] - C[idx] - iter;
            } else {
                A[idx] = (B[idx] + C[idx]) * (i - j);
            }
            
            /* Additional control flow to complicate SIMD transformation */
            if (A[idx] < 0) {
                A[idx] = abs(A[idx]) % 1000;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern */
        int idx = indices[i % size];
        
        /* Complex conditional with floating point operations */
        if (idx >= 0 && idx < size) {
            float temp = Y[idx] * 2.0f;
            
            if (temp > 100.0f) {
                X[i] = sinf(temp) * cosf(Y[idx]);
            } else if (temp < -50.0f) {
                X[i] = sqrtf(fabsf(temp)) + Y[idx];
            } else {
                X[i] = temp * 0.5f + Y[idx] * 0.3f;
            }
            
            /* SIMD-unfriendly operation */
            if (omp_get_thread_num() % 4 == 0) {
                X[i] = X[i] * 0.9f;
            }
        }
    }
}

/* Test 3: Nested parallel for simd inside target region */
void test_simt_conditional(double *D, int *mask, int nrows, int ncols) {
    #pragma omp target map(tofrom: D[0:nrows*ncols]) map(to: mask[0:nrows*ncols])
    {
        #pragma omp parallel for simd collapse(2) \
            simdlen(8) aligned(D:64) aligned(mask:64)
        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols; j++) {
                int idx = i * ncols + j;
                
                /* Condition depending on thread ID and mask */
                int tid = omp_get_thread_num();
                int cond = (tid + mask[idx]) % 7;
                
                switch (cond) {
                    case 0:
                        D[idx] = D[idx] * 2.0 + i - j;
                        break;
                    case 1:
                    case 2:
                        D[idx] = sqrt(D[idx] * D[idx] + 1.0);
                        break;
                    case 3:
                        D[idx] = (D[idx] < 0) ? -D[idx] : D[idx] * 0.5;
                        break;
                    default:
                        D[idx] = sin(D[idx]) * cos(j * 0.1);
                        if (tid % 2 == 0) {
                            D[idx] += 0.1;
                        }
                        break;
                }
            }
        }
    }
}

/* Test 4: Mixed constructs with device pointers */
void test_mixed_constructs(int *data, int *offsets, int n, int block_size) {
    /* First, a simple target teams distribute */
    #pragma omp target teams distribute \
        map(tofrom: data[0:n]) map(to: offsets[0:block_size]) \
        num_teams(8)
    for (int team = 0; team < 8; team++) {
        int start = team * block_size;
        int end = start + block_size;
        if (end > n) end = n;
        
        /* Nested parallel for simd within team */
        #pragma omp parallel for simd \
            simdlen(4) safelen(8)
        for (int i = start; i < end; i++) {
            /* Complex addressing with device pointers */
            int *ptr = &data[i];
            int offset = offsets[i % block_size];
            
            if (omp_get_team_num() % 2 == 0) {
                *ptr = (*ptr + offset) * 3;
            } else {
                *ptr = (*ptr - offset) / 2;
            }
            
            /* Additional condition to force control flow */
            if (i % 3 == 0 && omp_get_thread_num() % 2 == 0) {
                *ptr = (*ptr * 2) % 1000;
            }
        }
    }
}

/* Helper function to compute checksum */
long long compute_checksum(int *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + arr[i]) % CHECKSUM_MOD;
    }
    return sum;
}

long long compute_checksum_float(float *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + (long long)fabs(arr[i])) % CHECKSUM_MOD;
    }
    return sum;
}

long long compute_checksum_double(double *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + (long long)fabs(arr[i])) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int base_size = 1000;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    /* Dynamic allocations with varying sizes */
    int n = base_size;
    int m = base_size / 2;
    int total_int = n * m;
    int total_float = base_size * 2;
    int total_double = base_size * base_size / 4;
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *mask = (int *)malloc(total_double * sizeof(int));
    int *offsets = (int *)malloc(256 * sizeof(int));
    
    float *X = (float *)malloc(total_float * sizeof(float));
    float *Y = (float *)malloc(total_float * sizeof(float));
    int *indices = (int *)malloc(total_float * sizeof(int));
    
    double *D = (double *)malloc(total_double * sizeof(double));
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        C[i] = (i * 7) % 113;
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 59) * 0.1f;
        indices[i] = (i * 11) % total_float;
    }
    
    for (int i = 0; i < total_double; i++) {
        D[i] = (double)(i % 73) * 0.01;
        mask[i] = (i % 19) - 9;
    }
    
    for (int i = 0; i < 256; i++) {
        offsets[i] = (i * 5) % 31;
    }
    
    /* Execute test functions multiple times with different parameters */
    long long total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        if (iter % 4 == 0) {
            test_simt_nested(A, B, C, n, m, iter);
            total_checksum = (total_checksum + compute_checksum(A, total_int)) % CHECKSUM_MOD;
        }
        
        if (iter % 3 == 0) {
            test_simt_mapped(X, Y, indices, total_float, 1 + (iter % 4));
            total_checksum = (total_checksum + compute_checksum_float(X, total_float)) % CHECKSUM_MOD;
        }
        
        if (iter % 5 == 0) {
            test_simt_conditional(D, mask, base_size/2, base_size/2);
            total_checksum = (total_checksum + compute_checksum_double(D, total_double)) % CHECKSUM_MOD;
        }
        
        if (iter % 7 == 0) {
            test_mixed_constructs(A, offsets, total_int, 128);
            total_checksum = (total_checksum + compute_checksum(A, total_int)) % CHECKSUM_MOD;
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    
    /* Cleanup */
    free(A); free(B); free(C); free(mask); free(offsets);
    free(X); free(Y); free(indices);
    free(D);
    
    return 0;
}
