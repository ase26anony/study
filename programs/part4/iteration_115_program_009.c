#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        num_teams(64) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() % 4 == 0) && (i % 2 == 0)) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 0) {
                A[idx] = B[idx] - C[idx];
            } else {
                A[idx] = B[idx] + C[idx];
            }
            
            /* Additional control flow to complicate SIMD transformation */
            if (j % 8 == 0) {
                A[idx] += (omp_get_team_num() % 2) * 100;
            }
        }
    }
}

/* Test function 2: Pointer-based indirect accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, indices[0:size], Y[0:size*stride]) \
        map(tofrom: X[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i++) {
        int base_idx = indices[i] * stride;
        
        /* Complex pointer arithmetic and indirect access */
        float sum = 0.0f;
        for (int k = 0; k < 4; k++) {
            sum += Y[base_idx + k];
        }
        
        X[i] = sum * (i % 16) + (omp_get_thread_num() % 8);
        
        /* Conditional store based on SIMD lane */
        if ((i & 0xF) == 0) {  /* Check SIMD lane */
            X[i] *= 2.0f;
        }
    }
}

/* Test function 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols, mask[0:rows*cols]) \
        map(tofrom: D[0:rows*cols]) default(none)
    {
        #pragma omp teams distribute parallel for simd collapse(2) \
            num_teams(rows/16) thread_limit(256)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Complex condition depending on multiple factors */
                int thread_id = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if (mask[idx] > 0) {
                    if ((thread_id % 2) == (team_id % 3)) {
                        D[idx] = sin(D[idx]) * cos(D[idx] * 0.5);
                    } else {
                        D[idx] = sqrt(fabs(D[idx])) + 1.0;
                    }
                } else {
                    D[idx] = (thread_id % 8) * 0.125;
                }
                
                /* Early exit condition for some threads */
                if (thread_id % 16 == 0 && j > cols/2) {
                    D[idx] = -D[idx];
                }
            }
        }
    }
}

/* Test function 4: Mixed SIMD and non-SIMD constructs */
void test_mixed_simt(int *out, const int *in1, const int *in2, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: n, in1[0:n], in2[0:n]) map(tofrom: out[0:n])
    for (int i = 0; i < n; i++) {
        int temp = in1[i];
        
        /* Inner SIMD loop */
        #pragma omp simd reduction(+:temp) safelen(8)
        for (int j = 0; j < 8; j++) {
            temp += in2[(i + j) % n] * (j + 1);
        }
        
        /* Conditional with thread-dependent behavior */
        if (omp_get_thread_num() % 4 == 0) {
            temp *= 2;
        }
        
        out[i] = temp;
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *C, int *indices, 
                 float *X, float *Y, double *D, int *mask,
                 int n, int m, int total_size) {
    for (int i = 0; i < n * m; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
    }
    
    for (int i = 0; i < total_size; i++) {
        indices[i] = i % (total_size / 2);
        X[i] = (float)(i % 100) * 0.1f;
        Y[i] = (float)((i * 7) % 100) * 0.01f;
        D[i] = (double)(i % 50) * 0.5;
        mask[i] = (i % 7 == 0) ? 1 : 0;
    }
}

/* Compute checksum to verify execution */
unsigned long long compute_checksum(int *A, float *X, double *D, 
                                   int n, int total_size) {
    unsigned long long checksum = 0;
    
    for (int i = 0; i < n; i++) {
        checksum += (unsigned int)A[i];
    }
    
    for (int i = 0; i < total_size; i++) {
        checksum += (unsigned int)(X[i] * 1000);
        checksum += (unsigned long long)(D[i] * 1000);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int n = 512;
    int m = 256;
    int total_size = 10000;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            total_size = atoi(argv[3]);
        }
    }
    
    printf("Running with n=%d, m=%d, total_size=%d\n", n, m, total_size);
    
    /* Dynamic allocation */
    int *A = (int *)malloc(n * m * sizeof(int));
    int *B = (int *)malloc(n * m * sizeof(int));
    int *C = (int *)malloc(n * m * sizeof(int));
    
    int *indices = (int *)malloc(total_size * sizeof(int));
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    
    double *D = (double *)malloc(total_size * sizeof(double));
    int *mask = (int *)malloc(total_size * sizeof(int));
    
    int *out = (int *)malloc(total_size * sizeof(int));
    int *in1 = (int *)malloc(total_size * sizeof(int));
    int *in2 = (int *)malloc(total_size * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask || !out || !in1 || !in2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern-based data */
    init_arrays(A, B, C, indices, X, Y, D, mask, n, m, total_size);
    
    for (int i = 0; i < total_size; i++) {
        in1[i] = i % 73;
        in2[i] = (i * 5) % 89;
        out[i] = 0;
    }
    
    /* Execute test functions with different OpenMP constructs */
    printf("Executing test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    printf("Executing test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, total_size, 4);
    
    printf("Executing test_simt_conditional...\n");
    test_simt_conditional(D, mask, 100, total_size / 100);
    
    printf("Executing test_mixed_simt...\n");
    test_mixed_simt(out, in1, in2, total_size);
    
    /* Compute and print checksum */
    unsigned long long checksum = compute_checksum(A, X, D, n * m, total_size);
    printf("Final checksum: %llu\n", checksum);
    
    /* Verify some values to ensure execution */
    printf("Sample values: A[0]=%d, X[100]=%.2f, D[500]=%.2f, out[1000]=%d\n",
           A[0], X[100], D[500], out[1000]);
    
    /* Cleanup */
    free(A); free(B); free(C);
    free(indices); free(X); free(Y);
    free(D); free(mask);
    free(out); free(in1); free(in2);
    
    return 0;
}
