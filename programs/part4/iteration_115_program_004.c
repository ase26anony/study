#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() % 3 == 0) && (i % 2 == 0)) {
                A[idx] = B[idx] * C[idx] + i - j;
            } else if ((j % 4 == 0) && (omp_get_team_num() % 2 == 0)) {
                A[idx] = B[idx] + C[idx] * 2;
            } else {
                A[idx] = B[idx] - C[idx];
            }
            
            /* Additional conditional with early exit pattern */
            if (A[idx] < 0) {
                A[idx] = abs(A[idx]) % 256;
            }
        }
    }
}

/* Test function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size*stride]) map(to: Y[0:size*stride], indices[0:size]) \
        safelen(16)
    for (int i = 0; i < size; i++) {
        int idx = indices[i];
        /* Indirect access pattern - important for SIMT memory coalescing */
        X[i * stride] = Y[idx * stride] * 2.5f - X[(i + 1) % size * stride];
        
        /* Nested conditional with thread-dependent branching */
        if (omp_get_thread_num() % 2 == 0) {
            X[i * stride] += sinf(Y[idx * stride]) * 0.5f;
        } else {
            X[i * stride] -= cosf(Y[idx * stride]) * 0.3f;
        }
        
        /* Loop-carried dependency breaker with conditional */
        if (i > 0 && X[(i-1) * stride] > 100.0f) {
            X[i * stride] = X[i * stride] / 2.0f;
        }
    }
}

/* Test function 3: Multiple nested parallel regions with SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target teams distribute map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    for (int i = 0; i < rows; i++) {
        #pragma omp parallel for simd
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex conditional based on multiple thread queries */
            int thread_id = omp_get_thread_num();
            int team_id = omp_get_team_num();
            int num_threads = omp_get_num_threads();
            
            if ((thread_id % 4 == 0) && (team_id % 3 == 0)) {
                D[idx] = D[idx] * 3.14159 + thread_id;
            } else if ((mask[idx] % 5 == 0) && (num_threads > 4)) {
                D[idx] = D[idx] / 2.71828 - team_id;
            } else {
                D[idx] = sqrt(fabs(D[idx])) + thread_id * team_id;
            }
            
            /* Additional SIMD-friendly conditional */
            D[idx] = (D[idx] > 1000.0) ? D[idx] - 1000.0 : 
                    (D[idx] < -1000.0) ? D[idx] + 1000.0 : D[idx];
        }
    }
}

/* Test function 4: Mixed directives to explore different lowering paths */
void test_mixed_constructs(int *data, int n, int iter) {
    /* First target region: teams distribute simd */
    #pragma omp target teams distribute simd \
        map(tofrom: data[0:n]) \
        num_teams(n/32) thread_limit(64)
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2 + omp_get_team_num();
    }
    
    /* Second target region: teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n]) collapse(2) \
        private(iter)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (omp_get_thread_num() % 2 == (iter % 2)) {
                data[idx] += (i * j) % 256;
            }
        }
    }
}

/* Helper function to initialize arrays with pattern */
void init_arrays(int *A, int *B, int *C, int *indices, 
                 float *X, float *Y, double *D, int *mask,
                 int total_size) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = i % 97;
        B[i] = (i * 3) % 113;
        C[i] = (i + 7) % 151;
        indices[i] = (i * 5) % total_size;
        X[i] = sinf(i * 0.01f) * 100.0f;
        Y[i] = cosf(i * 0.02f) * 50.0f;
        D[i] = (i % 100) * 0.1;
        mask[i] = i % 11;
    }
}

/* Compute checksum to verify execution */
long long compute_checksum(int *A, float *X, double *D, int total_size) {
    long long checksum = 0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < total_size; i++) {
        checksum += (long long)A[i];
        checksum += (long long)(X[i] * 100);
        checksum += (long long)(D[i] * 1000);
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int n = 1000, m = 200;
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    int total_size = n * m;
    
    /* Dynamic allocation with pattern-based initialization */
    int *A = (int*)malloc(total_size * sizeof(int));
    int *B = (int*)malloc(total_size * sizeof(int));
    int *C = (int*)malloc(total_size * sizeof(int));
    int *indices = (int*)malloc(total_size * sizeof(int));
    float *X = (float*)malloc(total_size * sizeof(float));
    float *Y = (float*)malloc(total_size * sizeof(float));
    double *D = (double*)malloc(total_size * sizeof(double));
    int *mask = (int*)malloc(total_size * sizeof(int));
    
    if (!A || !B || !C || !indices || !X || !Y || !D || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant patterns */
    init_arrays(A, B, C, indices, X, Y, D, mask, total_size);
    
    printf("Starting OpenMP target offloading tests...\n");
    printf("Array size: %d x %d = %d elements\n", n, m, total_size);
    
    /* Execute test functions with different OpenMP target constructs */
    
    /* Test 1: Nested loops with collapse and SIMD */
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    /* Test 2: Pointer-based accesses with SIMD safelen */
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, total_size, 1);
    
    /* Test 3: Nested parallel for simd inside target teams */
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, n, m);
    
    /* Test 4: Mixed constructs */
    printf("Running test_mixed_constructs...\n");
    test_mixed_constructs(A, total_size, 5);
    
    /* Compute and print checksum */
    long long checksum = compute_checksum(A, X, D, total_size);
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(A); free(B); free(C); free(indices);
    free(X); free(Y); free(D); free(mask);
    
    return 0;
}
