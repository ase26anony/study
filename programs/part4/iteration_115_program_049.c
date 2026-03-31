#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional that depends on loop indices - may trigger SIMT transformation */
            if ((i + j) % 8 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i * j) % 5 == 0) {
                A[idx] = B[idx] / 2 + C[idx];
            } else {
                A[idx] = B[idx] + C[idx] + (i % 4);
            }
            
            /* Additional control flow with thread index */
            int tid = omp_get_thread_num();
            if (tid % 3 == 0) {
                A[idx] += iter;
            } else if (tid % 3 == 1) {
                A[idx] -= iter;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simd safelen(16)
    for (int i = 0; i < size; i += stride) {
        /* Indirect access pattern - encourages SIMT for memory coalescing */
        int idx = indices[i % size];
        float temp = Y[idx];
        
        /* Conditional based on indirect index */
        if (idx % 7 == 0) {
            X[i] = temp * 1.5f + sinf((float)i * 0.1f);
        } else if (idx % 3 == 0) {
            X[i] = temp * 0.75f + cosf((float)i * 0.05f);
        } else {
            X[i] = temp + (float)(i % 11) * 0.1f;
        }
        
        /* Additional SIMD-friendly but conditional operation */
        X[i] += (indices[(i + 1) % size] % 2 == 0) ? 0.5f : -0.5f;
    }
}

/* Test 3: Separate parallel and SIMD regions with thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp parallel for simd collapse(2)
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r * cols + c;
                
                /* Thread-dependent condition - may trigger IFN_GOMP_USE_SIMT */
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((tid + team_id) % 4 == 0) {
                    D[idx] = (mask[idx] > 0) ? D[idx] * 2.0 : D[idx] * 0.5;
                } else if ((tid * team_id) % 5 == 0) {
                    D[idx] = D[idx] + (double)(tid % 8);
                } else {
                    D[idx] = D[idx] - (double)(team_id % 6);
                }
                
                /* Nested condition based on computed value */
                if (D[idx] > 100.0) {
                    D[idx] = 100.0;
                } else if (D[idx] < -100.0) {
                    D[idx] = -100.0;
                }
            }
        }
    }
}

/* Test 4: Mixed directives to explore different lowering paths */
void test_mixed_simd(int *out, const int *in1, const int *in2, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: n, in1[0:n], in2[0:n]) map(tofrom: out[0:n])
    for (int i = 0; i < n; i++) {
        /* Complex expression with conditional */
        int val = in1[i] + in2[i];
        
        if (i % 16 < 8) {
            out[i] = val * (i % 8 + 1);
        } else {
            out[i] = val / ((i % 8) + 1);
        }
        
        /* Additional SIMD lane-dependent operation */
        out[i] += (i % 32) * ((i % 2 == 0) ? 1 : -1);
    }
}

/* Compute checksum to ensure all code paths executed */
long long compute_checksum(int *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + arr[i]) % CHECKSUM_MOD;
    }
    return sum;
}

long long compute_float_checksum(float *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + (long long)fabsf(arr[i] * 1000.0f)) % CHECKSUM_MOD;
    }
    return sum;
}

long long compute_double_checksum(double *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + (long long)fabs(arr[i] * 1000.0)) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 512;
    int m = 256;
    int iterations = 10;
    
    /* Parse command line arguments for flexibility */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d, iterations=%d\n", n, m, iterations);
    
    /* Allocate and initialize arrays with pattern-based data */
    int total_int = n * m;
    int total_small = n * 16;  /* For stride tests */
    
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *indices = (int *)malloc(total_small * sizeof(int));
    int *mask = (int *)malloc(total_int * sizeof(int));
    int *out = (int *)malloc(total_small * sizeof(int));
    int *in1 = (int *)malloc(total_small * sizeof(int));
    int *in2 = (int *)malloc(total_small * sizeof(int));
    
    float *X = (float *)malloc(total_small * sizeof(float));
    float *Y = (float *)malloc(total_small * sizeof(float));
    
    double *D = (double *)malloc(total_int * sizeof(double));
    
    if (!A || !B || !C || !indices || !mask || !out || !in1 || !in2 || !X || !Y || !D) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data (non-constant to prevent optimization) */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        mask[i] = (i % 7 == 0) ? 1 : 0;
        D[i] = (double)(i % 59) * 1.5;
    }
    
    for (int i = 0; i < total_small; i++) {
        indices[i] = (i * 5) % total_small;
        out[i] = 0;
        in1[i] = i % 73;
        in2[i] = (i * 7) % 89;
        X[i] = 0.0f;
        Y[i] = (float)(i % 43) * 0.7f;
    }
    
    long long checksum = 0;
    
    /* Execute multiple iterations to increase coverage chances */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        /* Test 1: Nested collapse with SIMD */
        test_simt_nested(A, B, C, n, m, iter);
        checksum = (checksum + compute_checksum(A, total_int)) % CHECKSUM_MOD;
        
        /* Test 2: Pointer-based with safelen */
        test_simt_mapped(X, Y, indices, total_small, 2);
        checksum = (checksum + compute_float_checksum(X, total_small)) % CHECKSUM_MOD;
        
        /* Test 3: Separate parallel and SIMD regions */
        test_simt_conditional(D, mask, n, m);
        checksum = (checksum + compute_double_checksum(D, total_int)) % CHECKSUM_MOD;
        
        /* Test 4: Mixed directives */
        test_mixed_simd(out, in1, in2, total_small);
        checksum = (checksum + compute_checksum(out, total_small)) % CHECKSUM_MOD;
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Verify some results to ensure execution */
    int verify_sum = 0;
    for (int i = 0; i < 100 && i < total_int; i++) {
        verify_sum += A[i];
    }
    printf("Sample verification sum (first 100 elements of A): %d\n", verify_sum);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices); free(mask);
    free(out); free(in1); free(in2); free(X); free(Y); free(D);
    
    return 0;
}
