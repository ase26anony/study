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

static int use_gpu_offload = 0;

/* Declare SIMD function for testing */
#pragma omp declare simd uniform(a, b) linear(i:1) notinbranch
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
        aligned(a, b, c: 64) linear(i:1) schedule(static)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) private(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_func(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp simd simdlen(4)
            for (int j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                c[idx] = a[idx] * 0.5 + b[idx] * 1.5;
            }
        }
    }
}

/* Test 5: Target simd with dynamic arrays and pointer access */
void test_dynamic_arrays(int n) {
    int *dyn_a = (int *)malloc(n * sizeof(int));
    int *dyn_b = (int *)malloc(n * sizeof(int));
    int *dyn_c = (int *)malloc(n * sizeof(int));
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        dyn_a[i] = i % 100;
        dyn_b[i] = (n - i) % 100;
    }
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: dyn_a[0:n], dyn_b[0:n]) map(from: dyn_c[0:n]) \
            simdlen(8) private(i)
        for (int i = 0; i < n; i++) {
            dyn_c[i] = dyn_a[i] + dyn_b[i] * 3;
        }
    }
    
    /* Compute checksum */
    long long sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += dyn_c[i];
    }
    printf("Dynamic arrays checksum: %lld\n", sum);
    
    free(dyn_a);
    free(dyn_b);
    free(dyn_c);
}

/* Test 6: SIMD with reduction clause */
float test_simd_reduction(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(4)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * arr[i];
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays */
    float *a_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *b_f = (float *)aligned_alloc(32, N * sizeof(float));
    float *c_f = (float *)aligned_alloc(32, N * sizeof(float));
    
    int *a_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *b_i = (int *)aligned_alloc(64, N * sizeof(int));
    int *c_i = (int *)aligned_alloc(64, N * sizeof(int));
    
    double *a_d = (double *)malloc(N * 16 * sizeof(double));
    double *b_d = (double *)malloc(N * 16 * sizeof(double));
    double *c_d = (double *)malloc(N * 16 * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N - i) * 0.2f;
        a_i[i] = i;
        b_i[i] = N - i;
    }
    
    for (int i = 0; i < N * 16; i++) {
        a_d[i] = i * 0.01;
        b_d[i] = i * 0.02;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    /* Compute checksum */
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c_f[i];
    }
    printf("Target SIMD checksum: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    /* Compute checksum */
    int checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < N; i++) {
        checksum2 += c_i[i];
    }
    printf("Parallel for SIMD checksum: %d\n", checksum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a_f, b_f, c_f, N/2, 2);
    
    /* Test 4: Mixed directives */
    printf("\nTest 4: Mixed directives\n");
    test_mixed_directives(a_d, b_d, c_d, N);
    
    /* Test 5: Dynamic arrays */
    printf("\nTest 5: Dynamic arrays\n");
    test_dynamic_arrays(N/4);
    
    /* Test 6: SIMD reduction */
    printf("\nTest 6: SIMD reduction\n");
    float red_sum = test_simd_reduction(a_f, N);
    printf("Reduction sum: %f\n", red_sum);
    
    /* Validation: Compare GPU and CPU paths if both were executed */
    if (use_gpu_offload) {
        /* Re-run test 1 without GPU offloading for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_cpu = (float *)aligned_alloc(32, N * sizeof(float));
        test_target_simd(a_f, b_f, c_cpu, N);
        
        /* Compare results */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_cpu[i]) > 0.001f) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at %d: GPU=%f, CPU=%f\n", 
                           i, c_f[i], c_cpu[i]);
                }
            }
        }
        
        if (errors == 0) {
            printf("\nValidation PASSED: GPU and CPU results match\n");
        } else {
            printf("\nValidation FAILED: %d mismatches found\n", errors);
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
