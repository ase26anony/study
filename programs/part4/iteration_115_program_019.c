#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <assert.h>

#define CHECKSUM_SEED 5381

/* Test function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional execution path that depends on thread/iteration */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((i + j) % 5 == 0) {
                A[idx] = B[idx] - C[idx] - iter;
            } else {
                A[idx] = B[idx] + C[idx];
            }
            
            /* Additional conditional with thread index check */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32) num_teams(8) thread_limit(256)
    for (int i = 0; i < size; i += stride) {
        /* Indirect access pattern - may influence SIMT memory coalescing */
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            /* Conditional store with floating point operations */
            if (Y[idx] > 0.5f) {
                X[i] = Y[idx] * 2.0f + (float)(i % 7);
            } else {
                X[i] = Y[idx] / 2.0f - (float)(i % 5);
            }
        }
        
        /* SIMD lane-dependent operation */
        int simd_lane = i % 32;
        X[i] += (float)simd_lane * 0.01f;
    }
}

/* Test function 3: Separate parallel and SIMD regions with conditional */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(4)
    {
        #pragma omp parallel for simd collapse(2) schedule(static, 16)
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r * cols + c;
                
                /* Thread-dependent conditional execution */
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((tid % 2) == (team_id % 2)) {
                    D[idx] = D[idx] * 1.5 + (double)mask[idx];
                } else {
                    D[idx] = D[idx] * 0.5 - (double)mask[idx];
                }
                
                /* Nested conditional inside SIMD loop */
                if (mask[idx] > 0 && D[idx] < 100.0) {
                    D[idx] += (double)((r + c) % 11);
                }
            }
        }
    }
}

/* Test function 4: Mixed SIMD and non-SIMD constructs */
void test_mixed_simd(int *out, const int *in1, const int *in2, int dim1, int dim2) {
    #pragma omp target teams distribute parallel for simd \
        map(to: dim1, dim2, in1[0:dim1*dim2], in2[0:dim1*dim2]) \
        map(tofrom: out[0:dim1*dim2]) \
        num_teams(dim1 > 1024 ? 32 : 16) thread_limit(64)
    for (int i = 0; i < dim1; i++) {
        /* Variable loop bound - prevents constant propagation */
        int limit = dim2 - (i % 7);
        for (int j = 0; j < limit; j++) {
            int idx = i * dim2 + j;
            
            /* Complex conditional chain */
            if (in1[idx] % 2 == 0) {
                out[idx] = in1[idx] + in2[idx];
                if (j % 3 == 0) {
                    out[idx] *= 2;
                }
            } else {
                out[idx] = in1[idx] - in2[idx];
                if (j % 4 == 0) {
                    out[idx] /= 2;
                }
            }
            
            /* SIMD lane conditional */
            if ((idx % 32) < 16) {
                out[idx] += 1;
            }
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
    /* Parse command line arguments for dynamic sizing */
    int n = 512;
    int m = 256;
    int iterations = 10;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    printf("Running SIMT tests with n=%d, m=%d, iterations=%d\n", n, m, iterations);
    
    /* Dynamic allocation with pattern-based initialization */
    size_t total_size = n * m;
    
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    
    double *D = (double *)malloc(total_size * sizeof(double));
    
    int *indices = (int *)malloc(total_size * sizeof(int));
    int *out_arr = (int *)malloc(total_size * sizeof(int));
    int *in1_arr = (int *)malloc(total_size * sizeof(int));
    int *in2_arr = (int *)malloc(total_size * sizeof(int));
    
    /* Pattern-based initialization */
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
        mask[i] = (i % 13) - 6;
        
        X[i] = (float)(i % 100) / 100.0f;
        Y[i] = (float)((i * 7) % 100) / 100.0f;
        
        D[i] = (double)(i % 200) / 2.0;
        
        indices[i] = (i * 11) % total_size;
        in1_arr[i] = i % 73;
        in2_arr[i] = (i * 5) % 89;
        out_arr[i] = 0;
    }
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        test_simt_nested(A, B, C, n, m, iter);
        
        test_simt_mapped(X, Y, indices, total_size, 1 + (iter % 4));
        
        test_simt_conditional(D, mask, n, m);
        
        test_mixed_simd(out_arr, in1_arr, in2_arr, n, m);
        
        /* Modify inputs slightly for next iteration */
        for (int i = 0; i < total_size; i += 100) {
            B[i] += 1;
            C[i] -= 1;
            Y[i] += 0.1f;
            mask[i] ^= 1;
        }
    }
    
    /* Compute and print checksums to verify execution */
    unsigned long checksum_A = compute_checksum(A, total_size * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, total_size * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, total_size * sizeof(double));
    unsigned long checksum_out = compute_checksum(out_arr, total_size * sizeof(int));
    
    printf("Checksums:\n");
    printf("  Array A: %lu\n", checksum_A);
    printf("  Array X: %lu\n", checksum_X);
    printf("  Array D: %lu\n", checksum_D);
    printf("  Array out: %lu\n", checksum_out);
    
    /* Cleanup */
    free(A); free(B); free(C); free(mask);
    free(X); free(Y);
    free(D);
    free(indices); free(out_arr); free(in1_arr); free(in2_arr);
    
    return 0;
}
