/* test_simt_transformation.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

/* Global flag to control offloading at runtime */
static int use_gpu_offload = 0;

/* Declare SIMD function for later use */
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(4)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(8) safelen(16) collapse(1) \
            private(n) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Host fallback - still SIMD but without target */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(4) safelen(8) collapse(1) \
        aligned(a, b, c: 64) linear(i:1) \
        schedule(static, 16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse and reduction */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd collapse(2) reduction(+:sum) \
        simdlen(4) safelen(8) private(i, j) \
        lastprivate(sum)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            c[idx] = a[idx] - b[idx];
            sum += c[idx];
        }
    }
    
    printf("Nested reduction sum: %f\n", sum);
}

/* Test 4: Mixed directives - simd inside parallel region */
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd nowait \
            simdlen(2) aligned(a, b, c: 32) \
            schedule(dynamic, 8)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] / (b[i] + 1.0);
        }
        
        #pragma omp barrier
        
        #pragma omp for simd simdlen(4)
        for (int i = 0; i < n; i++) {
            c[i] = c[i] * 2.0;
        }
    }
}

/* Test 5: Using declared SIMD function inside target region */
void test_declared_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) private(i)
        for (int i = 0; i < n; i++) {
            /* Call declared SIMD function */
            c[i] = simd_function(a[i], b[i], i);
        }
    } else {
        #pragma omp simd simdlen(4)
        for (int i = 0; i < n; i++) {
            c[i] = simd_function(a[i], b[i], i);
        }
    }
}

/* Test 6: Complex loop with multiple statements and conditions */
void test_complex_simd(int *a, int *b, int *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(32) collapse(1) \
        if(use_gpu_offload)
    for (int i = 0; i < n; i++) {
        int temp = a[i] + b[i];
        if (temp > 100) {
            c[i] = temp * 2;
        } else {
            c[i] = temp / 2;
        }
        c[i] += i % 8;  /* Add some lane-dependent computation */
    }
}

/* Compute checksum to prevent dead code elimination */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int compute_int_checksum(int *arr, int n) {
    int sum = 0;
    #pragma omp simd reduction(+:sum) simdlen(4)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays with dynamic allocation */
    float *a_f = (float*)malloc(N * sizeof(float));
    float *b_f = (float*)malloc(N * sizeof(float));
    float *c_f = (float*)malloc(N * sizeof(float));
    
    int *a_i = (int*)malloc(N * sizeof(int));
    int *b_i = (int*)malloc(N * sizeof(int));
    int *c_i = (int*)malloc(N * sizeof(int));
    
    double *a_d = (double*)malloc(N * sizeof(double));
    double *b_d = (double*)malloc(N * sizeof(double));
    double *c_d = (double*)malloc(N * sizeof(double));
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 1.5f;
        b_f[i] = N - i;
        a_i[i] = i;
        b_i[i] = (i * 3) % 7;
        a_d[i] = i * 0.5;
        b_d[i] = i * 2.0;
    }
    
    printf("Starting tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    printf("Checksum 1: %f\n", compute_checksum(c_f, N));
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    printf("Checksum 2: %d\n", compute_int_checksum(c_i, N));
    
    /* Test 3: Nested SIMD with reduction */
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a_f, b_f, c_f, N/2, 2);
    
    /* Test 4: Mixed directives */
    printf("\nTest 4: Mixed directives\n");
    test_mixed_directives(a_d, b_d, c_d, N);
    float sum_d = 0.0f;
    #pragma omp simd reduction(+:sum_d) simdlen(4)
    for (int i = 0; i < N; i++) {
        sum_d += (float)c_d[i];
    }
    printf("Checksum 4: %f\n", sum_d);
    
    /* Test 5: Declared SIMD function */
    printf("\nTest 5: Declared SIMD function\n");
    test_declared_simd(a_f, b_f, c_f, N);
    printf("Checksum 5: %f\n", compute_checksum(c_f, N));
    
    /* Test 6: Complex SIMD with conditions */
    printf("\nTest 6: Complex SIMD\n");
    test_complex_simd(a_i, b_i, c_i, N);
    printf("Checksum 6: %d\n", compute_int_checksum(c_i, N));
    
    /* Validation: Compare GPU and CPU results if both paths were tested */
    if (argc > 1) {
        printf("\nValidation complete.\n");
        
        /* Re-run test 1 without GPU for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        float *c_cpu = (float*)malloc(N * sizeof(float));
        test_target_simd(a_f, b_f, c_cpu, N);
        
        /* Simple comparison */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_cpu[i]) > 0.001f) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at %d: GPU=%f, CPU=%f\n", i, c_f[i], c_cpu[i]);
                }
            }
        }
        if (errors == 0) {
            printf("GPU and CPU results match perfectly.\n");
        } else {
            printf("Found %d differences between GPU and CPU results.\n", errors);
        }
        
        free(c_cpu);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a_f);
    free(b_f);
    free(c_f);
    free(a_i);
    free(b_i);
    free(c_i);
    free(a_d);
    free(b_d);
    free(c_d);
    
    return 0;
}
