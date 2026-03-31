#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional execution inside SIMD loop - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] - C[idx] + iter;
            } else {
                A[idx] = B[idx] + C[idx] - iter;
            }
            
            /* Additional conditional based on thread index */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32)
    for (int i = 0; i < size; i++) {
        /* Indirect memory access - may influence SIMT transformation decisions */
        int idx = indices[i] % size;
        
        /* Complex conditional with floating point operations */
        if (X[i] > 0.0f) {
            X[i] = Y[idx] * scale + sinf((float)i * 0.01f);
        } else {
            X[i] = Y[idx] / scale - cosf((float)i * 0.01f);
        }
        
        /* Nested conditional based on computed value */
        if (fabsf(X[i]) < 0.5f) {
            X[i] *= 2.0f;
        }
    }
}

/* Test function 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int offset) {
    #pragma omp target map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(8)
    {
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            #pragma omp for simd collapse(2) nowait
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int idx = i * cols + j;
                    
                    /* Conditional that depends on thread/team IDs */
                    if ((tid + team_id) % 2 == 0) {
                        D[idx] = (mask[idx] % 2 == 0) ? 
                                 D[idx] * 1.5 + offset : 
                                 D[idx] * 0.5 - offset;
                    } else {
                        D[idx] = (mask[idx] % 3 == 0) ?
                                 D[idx] * 2.0 + offset :
                                 D[idx] * 0.75 - offset;
                    }
                    
                    /* Additional SIMD-friendly conditional */
                    D[idx] = (D[idx] > 100.0) ? 100.0 : 
                            ((D[idx] < -100.0) ? -100.0 : D[idx]);
                }
            }
        }
    }
}

/* Test function 4: Mixed SIMD and non-SIMD constructs */
void test_mixed_constructs(int *data, int *pattern, int n, int m) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) map(to: pattern[0:m])
    for (int i = 0; i < n; i++) {
        int sum = 0;
        
        /* Inner loop that might be SIMD-ized */
        #pragma omp simd reduction(+:sum) safelen(16)
        for (int j = 0; j < m; j++) {
            sum += pattern[j % m] * (i + j);
        }
        
        data[i] = sum % 256;
        
        /* Conditional that might trigger SIMT transformation */
        if (data[i] > 128) {
            data[i] = 255 - data[i];
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(int *A, int *B, int *C, int *indices, float *X, float *Y, 
                 double *D, int *mask, int *pattern, int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = i % 97;
        B[i] = (i * 3) % 101;
        C[i] = (i * 5) % 103;
        indices[i] = (i * 7) % total_size;
        X[i] = sinf(i * 0.1f);
        Y[i] = cosf(i * 0.1f);
        D[i] = (i % 2 == 0) ? i * 0.5 : -i * 0.5;
        mask[i] = i % 11;
        pattern[i % 256] = i % 19;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int *data, int size) {
    long long checksum = 0;
    
    #pragma omp parallel for simd reduction(+:checksum)
    for (int i = 0; i < size; i++) {
        checksum += (long long)A[i];
        checksum += (long long)(X[i] * 1000);
        checksum += (long long)(D[i] * 1000);
        if (i < size / 4) {
            checksum += data[i];
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int n = 512, m = 256, iterations = 10;
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    if (n > MAX_SIZE) n = MAX_SIZE;
    if (m > MAX_SIZE) m = MAX_SIZE;
    
    int total_size = n * m;
    int pattern_size = 256;
    
    printf("Running with n=%d, m=%d, total_size=%d, iterations=%d\n", 
           n, m, total_size, iterations);
    
    /* Dynamic allocation */
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    int *C = (int*)malloc(total_size * sizeof(int));
    int *indices = (int*)malloc(total_size * sizeof(int));
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size * sizeof(float));
    double *D = (double*)malloc(total_size * sizeof(double));
    int *mask = (int*)malloc(total_size * sizeof(int));
    int *pattern = (int*)malloc(pattern_size * sizeof(int));
    int *data = (int*)malloc((total_size / 4) * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask || !pattern || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(A, B, C, indices, X, Y, D, mask, pattern, total_size);
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        /* Test 1: Nested loops with collapse */
        test_simt_nested(A, B, C, n, m, iter);
        
        /* Test 2: Pointer-based accesses */
        test_simt_mapped(X, Y, indices, total_size, 1.0f + iter * 0.1f);
        
        /* Test 3: Conditional execution */
        test_simt_conditional(D, mask, n, m, iter);
        
        /* Test 4: Mixed constructs */
        test_mixed_constructs(data, pattern, total_size / 4, pattern_size);
        
        /* Verify intermediate results */
        if ((iter + 1) % 5 == 0) {
            long long checksum = compute_checksum(A, X, D, data, total_size / 4);
            printf("  Intermediate checksum: %lld\n", checksum);
        }
    }
    
    /* Final verification */
    long long final_checksum = compute_checksum(A, X, D, data, total_size / 4);
    printf("\nFinal checksum: %lld\n", final_checksum);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices);
    free(X); free(Y); free(D); free(mask);
    free(pattern); free(data);
    
    return 0;
}
