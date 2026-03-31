#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() % 3 == 0) && (i % 2 == 0)) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((j % 4 == 0) && (omp_get_team_num() % 2 == 0)) {
                A[idx] = B[idx] / 2 + C[idx];
            } else {
                A[idx] = B[idx] + C[idx] + (i * j);
            }
            
            /* Additional conditional with early exit pattern */
            if (A[idx] > 1000) {
                A[idx] = A[idx] % 1000;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD clause */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern - important for SIMT transformation */
        int idx = indices[i];
        
        /* Complex conditional execution */
        if (idx >= 0 && idx < size) {
            float temp = Y[idx];
            
            /* Nested conditionals to create control flow divergence */
            if (temp > 0.5f) {
                X[i] = sqrtf(temp) * (omp_get_thread_num() % 8 + 1);
            } else if (temp < -0.5f) {
                X[i] = temp * temp * (omp_get_team_num() % 4 + 1);
            } else {
                X[i] = temp + (i % 100) * 0.01f;
            }
            
            /* Additional SIMD-friendly but conditional operation */
            X[i] = (X[i] > 100.0f) ? 100.0f : ((X[i] < -100.0f) ? -100.0f : X[i]);
        } else {
            X[i] = 0.0f;
        }
    }
}

/* Test 3: Nested parallel region with SIMD inside target */
void test_simt_conditional(double *D, int *mask, int nrows, int ncols) {
    #pragma omp target map(to: nrows, ncols) map(tofrom: D[0:nrows*ncols]) map(to: mask[0:nrows*ncols])
    {
        #pragma omp parallel for simd collapse(2)
        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols; j++) {
                int idx = i * ncols + j;
                
                /* Complex condition depending on multiple factors */
                int thread_id = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((mask[idx] % 2 == 0) && (thread_id % 3 == 0)) {
                    D[idx] = sin(D[idx]) * cos(team_id * 0.1);
                } else if ((mask[idx] % 3 == 0) && (team_id % 2 == 0)) {
                    D[idx] = exp(D[idx] * 0.01) - 1.0;
                } else if ((i + j) % 5 == 0) {
                    D[idx] = D[idx] * (1.0 + (thread_id % 10) * 0.001);
                } else {
                    D[idx] = D[idx] + (i * 0.01) - (j * 0.005);
                }
                
                /* SIMD lane-dependent operation */
                if (D[idx] > 1.0) {
                    D[idx] = log(D[idx]);
                }
            }
        }
    }
}

/* Test 4: Multiple SIMD clauses with different parameters */
void test_multi_simd(int *out, const int *in1, const int *in2, int len, int block) {
    #pragma omp target teams distribute parallel for simd \
        map(to: len, block, in1[0:len], in2[0:len]) map(tofrom: out[0:len]) \
        num_teams(len/block) thread_limit(256)
    for (int i = 0; i < len; i++) {
        /* Data-dependent conditional with thread/team awareness */
        int tid = omp_get_thread_num();
        int bid = omp_get_team_num();
        
        if ((bid * 256 + tid) % 7 == 0) {
            out[i] = in1[i] * in2[i] + tid;
        } else if ((i % block) == 0) {
            out[i] = in1[i] + in2[i] * bid;
        } else {
            out[i] = in1[i] - in2[i] + (tid % 16);
        }
        
        /* Additional conditional to create more control flow */
        if (out[i] < 0) {
            out[i] = (out[i] % 1000) + 1000;
        }
    }
}

/* Helper function to initialize arrays with pattern */
void init_arrays(int *A, int *B, int *C, int *indices, float *X, float *Y, 
                 double *D, int *mask, int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        indices[i] = (i * 7) % total_size;
        X[i] = sinf(i * 0.01f) * 100.0f;
        Y[i] = cosf(i * 0.02f) * 50.0f;
        D[i] = (i % 1000) * 0.001;
        mask[i] = i % 11;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int *out, int total_size) {
    long long sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < total_size; i++) {
        sum += A[i];
        sum += (long long)(X[i] * 100);
        sum += (long long)(D[i] * 1000);
        if (i < total_size / 2) {
            sum += out[i];
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 512, m = 256;  /* Default sizes */
    int total_size = n * m;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        total_size = n * m;
    }
    if (total_size > MAX_SIZE) {
        total_size = MAX_SIZE;
        n = sqrt(MAX_SIZE);
        m = MAX_SIZE / n;
    }
    
    printf("Testing SIMT transformation with n=%d, m=%d, total_size=%d\n", n, m, total_size);
    
    /* Dynamic allocation */
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    double *D = (double *)malloc(total_size * sizeof(double));
    int *mask = (int *)malloc(total_size * sizeof(int));
    int *out = (int *)malloc(total_size * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask || !out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    init_arrays(A, B, C, indices, X, Y, D, mask, total_size);
    
    printf("Starting OpenMP target offload tests...\n");
    
    /* Execute test functions with different OpenMP constructs */
    test_simt_nested(A, B, C, n, m);
    
    test_simt_mapped(X, Y, indices, total_size, 2);
    
    test_simt_conditional(D, mask, n, m);
    
    test_multi_simd(out, B, C, total_size, 64);
    
    /* Compute and print checksum */
    long long checksum = compute_checksum(A, X, D, out, total_size);
    printf("Final checksum: %lld\n", checksum);
    
    /* Verify some values */
    int verify_count = 0;
    #pragma omp parallel for reduction(+:verify_count)
    for (int i = 0; i < total_size; i += total_size/10) {
        if (A[i] != 0 || out[i] != 0) {
            verify_count++;
        }
    }
    printf("Non-zero elements in sampled positions: %d\n", verify_count);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices);
    free(X); free(Y); free(D); free(mask); free(out);
    
    return 0;
}
