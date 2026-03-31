#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

// Global flag to control GPU offloading
int use_gpu_offload = 0;

// Function with declare simd pragma
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        #pragma omp simd simdlen(4) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

// Test 2: Parallel for SIMD with various clauses
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        aligned(a, b, c: 64) linear(i:1) private(i)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

// Test 3: Nested SIMD with collapse
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_func(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Teams distribute with SIMD
void test_teams_distribute_simd(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
        simdlen(2) num_teams(4) thread_limit(128)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] / (b[i] + 1.0);
    }
}

// Test 5: Mixed directives with dynamic arrays
void test_mixed_directives(int **matrix_a, int **matrix_b, int **matrix_c, 
                          int rows, int cols) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) safelen(8)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                matrix_c[i][j] = matrix_a[i][j] + matrix_b[i][j] * 2;
            }
        }
    }
}

// Test 6: Reduction with SIMD
float test_reduction_simd(float *data, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[i];
    }
    
    return sum;
}

// Test 7: Conditional SIMD with runtime check
void test_conditional_simd(float *a, float *b, float *c, int n, int threshold) {
    // This mimics the IFN_GOMP_USE_SIMT check
    if (threshold > 0) {
        #pragma omp target simd map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) if(target: use_gpu_offload)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] - b[i];
        }
    } else {
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            c[i] = a[i] - b[i];
        }
    }
}

int main(int argc, char *argv[]) {
    // Parse command line argument
    if (argc > 1 && strcmp(argv[1], "--use-gpu") == 0) {
        use_gpu_offload = 1;
        printf("GPU offloading enabled\n");
    } else {
        printf("GPU offloading disabled\n");
    }
    
    // Allocate and initialize arrays
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    
    int *ia = (int*)malloc(N * sizeof(int));
    int *ib = (int*)malloc(N * sizeof(int));
    int *ic = (int*)malloc(N * sizeof(int));
    
    double *da = (double*)malloc(N * sizeof(double));
    double *db = (double*)malloc(N * sizeof(double));
    double *dc = (double*)malloc(N * sizeof(double));
    
    // Initialize with patterned data
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
        
        ia[i] = i % 100;
        ib[i] = (i * 2) % 100;
        ic[i] = 0;
        
        da[i] = (double)i * 0.5;
        db[i] = (double)(N - i) * 0.5;
        dc[i] = 0.0;
    }
    
    // Allocate 2D arrays for mixed directives test
    int rows = M, cols = M;
    int **matrix_a = (int**)malloc(rows * sizeof(int*));
    int **matrix_b = (int**)malloc(rows * sizeof(int*));
    int **matrix_c = (int**)malloc(rows * sizeof(int*));
    
    for (int i = 0; i < rows; i++) {
        matrix_a[i] = (int*)malloc(cols * sizeof(int));
        matrix_b[i] = (int*)malloc(cols * sizeof(int));
        matrix_c[i] = (int*)malloc(cols * sizeof(int));
        
        for (int j = 0; j < cols; j++) {
            matrix_a[i][j] = i * cols + j;
            matrix_b[i][j] = (i * cols + j) * 2;
            matrix_c[i][j] = 0;
        }
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    // Test 1: Target SIMD with conditional execution
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(a, b, c, N);
    
    // Compute checksum
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c[i];
    }
    printf("Checksum 1: %f\n", checksum1);
    
    // Test 2: Parallel for SIMD
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(ia, ib, ic, N);
    
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += ic[i];
    }
    printf("Checksum 2: %d\n", checksum2);
    
    // Test 3: Nested SIMD with collapse
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(a, b, d, N/2, 2);
    
    float checksum3 = 0.0f;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N; i++) {
        checksum3 += d[i];
    }
    printf("Checksum 3: %f\n", checksum3);
    
    // Test 4: Teams distribute SIMD
    printf("\nTest 4: Teams distribute SIMD\n");
    test_teams_distribute_simd(da, db, dc, N);
    
    double checksum4 = 0.0;
    #pragma omp simd reduction(+:checksum4)
    for (int i = 0; i < N; i++) {
        checksum4 += dc[i];
    }
    printf("Checksum 4: %lf\n", checksum4);
    
    // Test 5: Mixed directives
    printf("\nTest 5: Mixed directives\n");
    test_mixed_directives(matrix_a, matrix_b, matrix_c, rows, cols);
    
    int checksum5 = 0;
    for (int i = 0; i < rows; i++) {
        #pragma omp simd reduction(+:checksum5)
        for (int j = 0; j < cols; j++) {
            checksum5 += matrix_c[i][j];
        }
    }
    printf("Checksum 5: %d\n", checksum5);
    
    // Test 6: Reduction SIMD
    printf("\nTest 6: Reduction SIMD\n");
    float reduction_result = test_reduction_simd(a, N);
    printf("Reduction result: %f\n", reduction_result);
    
    // Test 7: Conditional SIMD
    printf("\nTest 7: Conditional SIMD\n");
    test_conditional_simd(a, b, c, N, use_gpu_offload);
    
    float checksum7 = 0.0f;
    #pragma omp simd reduction(+:checksum7)
    for (int i = 0; i < N; i++) {
        checksum7 += c[i];
    }
    printf("Checksum 7: %f\n", checksum7);
    
    // Validation: Compare GPU and CPU paths if both were executed
    if (use_gpu_offload) {
        // Re-run test 1 without GPU offloading for comparison
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_cpu = (float*)malloc(N * sizeof(float));
        test_target_simd(a, b, c_cpu, N);
        
        // Compare results
        int errors = 0;
        #pragma omp simd reduction(+:errors)
        for (int i = 0; i < N; i++) {
            if (fabs(c[i] - c_cpu[i]) > 1e-6) {
                errors++;
            }
        }
        
        if (errors == 0) {
            printf("\nValidation: GPU and CPU results match!\n");
        } else {
            printf("\nValidation: Found %d differences between GPU and CPU\n", errors);
        }
        
        free(c_cpu);
        use_gpu_offload = saved_flag;
    }
    
    // Cleanup
    free(a); free(b); free(c); free(d);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    
    for (int i = 0; i < rows; i++) {
        free(matrix_a[i]);
        free(matrix_b[i]);
        free(matrix_c[i]);
    }
    free(matrix_a);
    free(matrix_b);
    free(matrix_c);
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
