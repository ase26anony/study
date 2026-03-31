#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() % 3 == 0) && (i % 2 == 0)) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((j % 4 == 0) && (omp_get_team_num() % 2 == 0)) {
                A[idx] = B[idx] - C[idx] / 2;
            } else {
                /* Complex expression to prevent optimization */
                A[idx] = (B[idx] * C[idx]) % 256 + (i * j) % 128;
            }
            
            /* Additional conditional with early exit pattern */
            if (A[idx] > 200) {
                A[idx] = A[idx] % 100;
            }
        }
    }
}

/* Test 2: Mapped pointers with indirect accesses and SIMD safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(16)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern */
        int idx = indices[i % size];
        
        /* Complex conditional with floating point operations */
        if (idx % 5 == 0) {
            X[i] = Y[idx] * 2.5f + sinf((float)i * 0.1f);
        } else if (idx % 3 == 0 && omp_get_thread_num() % 2 == 0) {
            X[i] = sqrtf(fabsf(Y[idx])) * (omp_get_team_num() % 4 + 1);
        } else {
            X[i] = Y[idx] + cosf((float)idx * 0.05f);
        }
        
        /* Nested condition to create more complex CFG */
        if (X[i] > 10.0f) {
            X[i] = 10.0f;
        } else if (X[i] < -10.0f) {
            X[i] = -10.0f;
        }
    }
}

/* Test 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int n, int block_size) {
    #pragma omp target map(tofrom: D[0:n]) map(to: mask[0:n]) \
        defaultmap(tofrom:scalar)
    {
        #pragma omp teams distribute
        for (int block = 0; block < n; block += block_size) {
            int end = (block + block_size < n) ? block + block_size : n;
            
            #pragma omp parallel for simd
            for (int i = block; i < end; i++) {
                /* Condition depending on thread index and mask */
                int tid = omp_get_thread_num();
                int team = omp_get_team_num();
                
                if (mask[i] == 1) {
                    if ((tid % 4 == 0) && (team % 2 == 0)) {
                        D[i] = D[i] * 3.14159 + (double)(i % 100);
                    } else {
                        D[i] = D[i] / 2.0 + (double)(tid % 50);
                    }
                } else {
                    D[i] = (double)((i + tid) % 255) * 0.5;
                }
                
                /* Additional conditional with goto-like pattern */
                if (D[i] > 1000.0) {
                    D[i] = 1000.0;
                }
            }
        }
    }
}

/* Test 4: Mixed SIMD and non-SIMD constructs */
void test_mixed_simd(int *result, int *src1, int *src2, int dim1, int dim2) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: result[0:dim1*dim2]) map(to: src1[0:dim1*dim2], src2[0:dim1*dim2])
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            
            /* Switch-like conditional structure */
            int cond = (i + j) % 6;
            switch (cond) {
                case 0:
                    result[idx] = src1[idx] + src2[idx];
                    break;
                case 1:
                    result[idx] = src1[idx] - src2[idx];
                    break;
                case 2:
                    result[idx] = src1[idx] * src2[idx];
                    break;
                case 3:
                    result[idx] = (src1[idx] + omp_get_thread_num()) % 256;
                    break;
                default:
                    result[idx] = (src2[idx] + omp_get_team_num()) % 128;
                    break;
            }
            
            /* SIMD pragma inside the loop body (nested) */
            #pragma omp simd
            for (int k = 0; k < 4; k++) {
                if (k % 2 == 0) {
                    result[idx] += k;
                }
            }
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *C, int *mask, float *X, float *Y, 
                 double *D, int *indices, int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        C[i] = (i * 7) % 113;
        mask[i] = (i % 7 == 0) ? 1 : 0;
        X[i] = (float)(i % 100) * 0.1f;
        Y[i] = (float)((i * 11) % 200) * 0.05f;
        D[i] = (double)(i % 300) * 0.01;
        indices[i] = (i * 13) % total_size;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int total_size) {
    long long sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < total_size; i++) {
        sum += (long long)A[i];
        sum += (long long)(X[i] * 100);
        sum += (long long)(D[i] * 1000);
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 512;
    int m = 256;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (n <= 0) n = 512;
        if (m <= 0) m = 256;
    }
    
    int total_size = n * m;
    if (total_size > MAX_SIZE) {
        total_size = MAX_SIZE;
        n = m = (int)sqrt(MAX_SIZE);
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d (total=%d)\n", n, m, total_size);
    
    /* Dynamic allocation */
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    int *result = (int *)malloc(total_size * sizeof(int));
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    double *D = (double *)malloc(total_size * sizeof(double));
    
    if (!A || !B || !C || !mask || !indices || !result || !X || !Y || !D) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern-based data */
    init_arrays(A, B, C, mask, X, Y, D, indices, total_size);
    
    printf("Starting OpenMP target offloading tests...\n");
    
    /* Execute test functions with different OpenMP constructs */
    test_simt_nested(A, B, C, n, m);
    printf("  test_simt_nested completed\n");
    
    test_simt_mapped(X, Y, indices, total_size, 2);
    printf("  test_simt_mapped completed\n");
    
    test_simt_conditional(D, mask, total_size, 64);
    printf("  test_simt_conditional completed\n");
    
    test_mixed_simd(result, B, C, n, m);
    printf("  test_mixed_simd completed\n");
    
    /* Compute and print checksum */
    long long checksum = compute_checksum(A, X, D, total_size);
    printf("Final checksum: %lld\n", checksum);
    
    /* Verify some values */
    int verify_count = (total_size < 10) ? total_size : 10;
    printf("First %d values of A: ", verify_count);
    for (int i = 0; i < verify_count; i++) {
        printf("%d ", A[i]);
    }
    printf("\n");
    
    /* Cleanup */
    free(A); free(B); free(C); free(mask); free(indices); free(result);
    free(X); free(Y); free(D);
    
    return 0;
}
