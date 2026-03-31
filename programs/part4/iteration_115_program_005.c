#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 100000

/* Test function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        num_teams(128) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional inside SIMD loop - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2;
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] + 1;
            } else {
                A[idx] = B[idx] - 1;
            }
            
            /* Additional control flow with thread index */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += (i % 7);
            }
        }
    }
}

/* Test function 2: Pointer-based indirect accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i++) {
        /* Indirect access pattern - encourages memory coalescing analysis */
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            X[i] = Y[idx] * 3.14f;
            
            /* Complex conditional with floating point */
            if (X[i] > 100.0f) {
                X[i] = sqrtf(X[i]);
            } else if (X[i] < -50.0f) {
                X[i] = fabsf(X[i]);
            }
        }
        
        /* Thread-dependent computation */
        int team_id = omp_get_team_num();
        if (team_id % 2 == 0) {
            X[i] += 0.5f;
        }
    }
}

/* Test function 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(int *data, int *mask, int rows, int cols) {
    #pragma omp target map(tofrom: data[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(64)
    {
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            
            #pragma omp for simd collapse(2) nowait
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    int idx = r * cols + c;
                    
                    /* Conditional based on thread ID - may trigger SIMT branching */
                    if (tid % 2 == 0) {
                        data[idx] = mask[idx] * data[idx];
                    } else {
                        data[idx] = mask[idx] + data[idx];
                    }
                    
                    /* Nested conditionals */
                    if (data[idx] > 1000) {
                        data[idx] %= 1000;
                    }
                    
                    /* SIMD-unfriendly pattern to encourage transformation */
                    for (int k = 0; k < 2; k++) {
                        if (k == (tid % 2)) {
                            data[idx] += k;
                        }
                    }
                }
            }
        }
    }
}

/* Test function 4: Mixed SIMD and non-SIMD constructs */
void test_mixed_simd(double *vec1, double *vec2, int n, int iter) {
    for (int it = 0; it < iter; it++) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: vec1[0:n], vec2[0:n]) \
            reduction(+:vec1[0:n])
        for (int i = 0; i < n; i++) {
            /* Complex computation with conditionals */
            double temp = vec1[i] * vec2[i];
            
            if (temp > 0.0) {
                vec1[i] = sin(temp) * cos(vec2[i]);
            } else {
                vec1[i] = exp(-fabs(temp));
            }
            
            /* Thread/team dependent operation */
            if ((omp_get_thread_num() + omp_get_team_num()) % 3 == 0) {
                vec1[i] *= 1.1;
            }
        }
    }
}

/* Helper function to compute checksum */
long long compute_checksum(int *arr, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + arr[i]) % 1000000007;
    }
    return sum;
}

double compute_fchecksum(float *arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] * (i + 1);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 1000, m = 100;
    int iter = 5;
    
    /* Parse command line arguments */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iter = atoi(argv[3]);
        }
    }
    
    printf("Running with n=%d, m=%d, iterations=%d\n", n, m, iter);
    
    /* Dynamic allocation */
    int total_size = n * m;
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    
    double *vec1 = (double *)malloc(n * sizeof(double));
    double *vec2 = (double *)malloc(n * sizeof(double));
    
    if (!A || !B || !mask || !indices || !X || !Y || !vec1 || !vec2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern-based data */
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        mask[i] = (i % 13) * ((i % 7) + 1);
        indices[i] = (i * 17) % total_size;
        X[i] = (float)(i % 53) * 0.5f;
        Y[i] = (float)(i % 71) * 0.3f;
    }
    
    for (int i = 0; i < n; i++) {
        vec1[i] = (double)(i % 41) * 0.1;
        vec2[i] = (double)(i % 59) * 0.2;
    }
    
    printf("Initial checksum A: %lld\n", compute_checksum(A, total_size));
    printf("Initial checksum X: %f\n", compute_fchecksum(X, total_size));
    
    /* Execute test functions */
    printf("\nRunning test_simt_nested...\n");
    test_simt_nested(A, B, n, m);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, total_size);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(A, mask, n, m);
    
    printf("Running test_mixed_simd...\n");
    test_mixed_simd(vec1, vec2, n, iter);
    
    /* Compute final checksums */
    long long final_checksum_A = compute_checksum(A, total_size);
    double final_checksum_X = compute_fchecksum(X, total_size);
    double final_sum_vec1 = 0.0;
    
    for (int i = 0; i < n; i++) {
        final_sum_vec1 += vec1[i];
    }
    
    printf("\nFinal results:\n");
    printf("Checksum A: %lld\n", final_checksum_A);
    printf("Checksum X: %f\n", final_checksum_X);
    printf("Sum vec1: %f\n", final_sum_vec1);
    
    /* Cleanup */
    free(A);
    free(B);
    free(mask);
    free(indices);
    free(X);
    free(Y);
    free(vec1);
    free(vec2);
    
    return 0;
}
