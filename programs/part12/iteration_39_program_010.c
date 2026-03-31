/* test_simt_transformation.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test_simt_transformation.c -o test_simt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

/* Global flag to control offloading at runtime */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(8)
float simd_function(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                simdlen(4) safelen(8) private(i) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c:32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
            aligned(a, b, c:64) linear(i:1) private(i) \
            schedule(static, 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) private(i, j) \
                aligned(a, b, c:32)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] - b[idx] / (j + 1);
            }
        }
    }
}

/* Test 4: Reduction with SIMD */
float test_simd_reduction(float *a, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8) safelen(16) \
            aligned(a:32) linear(i:1)
    for (int i = 0; i < n; i++) {
        sum += a[i] * a[i];
    }
    
    return sum;
}

/* Test 5: Mixed directives with declare simd function calls */
void test_mixed_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                simdlen(4) num_teams(2)
        for (int i = 0; i < n; i++) {
            /* Call declare simd function inside SIMD region */
            c[i] = simd_function(a[i], b[i], i);
        }
    } else {
        #pragma omp parallel for simd simdlen(8) private(i)
        for (int i = 0; i < n; i++) {
            c[i] = simd_function(a[i], b[i], i);
        }
    }
}

/* Test 6: Dynamic pointer-based accesses with SIMD */
void test_pointer_simd(float **a, float **b, float **c, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
            map(to: a[0:n][0:m], b[0:n][0:m]) map(from: c[0:n][0:m]) \
            simdlen(4) safelen(8) private(i, j) \
            num_teams(4) thread_limit(64)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            c[i][j] = a[i][j] * 2.0f + b[i][j];
        }
    }
}

/* Initialize arrays with patterned data */
void init_arrays(float *a, float *b, int *ia, int *ib, int n) {
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < n; i++) {
        a[i] = i * 1.5f;
        b[i] = (n - i) * 0.75f;
        ia[i] = i % 100;
        ib[i] = (i * 2) % 100;
    }
}

/* Compute checksum to prevent dead code elimination */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
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
    
    /* Allocate arrays with different alignments */
    float *a = (float*)aligned_alloc(32, N * sizeof(float));
    float *b = (float*)aligned_alloc(32, N * sizeof(float));
    float *c1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *c2 = (float*)aligned_alloc(32, N * sizeof(float));
    float *c3 = (float*)aligned_alloc(32, N * M * sizeof(float));
    
    int *ia = (int*)aligned_alloc(64, N * sizeof(int));
    int *ib = (int*)aligned_alloc(64, N * sizeof(int));
    int *ic = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Initialize data */
    init_arrays(a, b, ia, ib, N);
    
    float checksum = 0.0f;
    
    /* Test 1: Target SIMD with conditional execution */
    printf("Test 1: Target SIMD\n");
    test_target_simd(a, b, c1, N);
    checksum += compute_checksum(c1, N);
    printf("  Checksum 1: %f\n", compute_checksum(c1, N));
    
    /* Test 2: Parallel for SIMD */
    printf("Test 2: Parallel for SIMD\n");
    test_parallel_for_simd(ia, ib, ic, N);
    checksum += compute_checksum((float*)ic, N);
    printf("  Checksum 2: %f\n", compute_checksum((float*)ic, N));
    
    /* Test 3: Nested SIMD */
    printf("Test 3: Nested SIMD\n");
    test_nested_simd(a, b, c3, N, M);
    checksum += compute_checksum(c3, N * M);
    printf("  Checksum 3: %f\n", compute_checksum(c3, N * M));
    
    /* Test 4: SIMD reduction */
    printf("Test 4: SIMD reduction\n");
    float reduction_sum = test_simd_reduction(a, N);
    checksum += reduction_sum;
    printf("  Reduction sum: %f\n", reduction_sum);
    
    /* Test 5: Mixed directives with declare simd */
    printf("Test 5: Mixed directives\n");
    test_mixed_simd(a, b, c2, N);
    checksum += compute_checksum(c2, N);
    printf("  Checksum 5: %f\n", compute_checksum(c2, N));
    
    /* Test 6: Pointer-based SIMD (only if GPU offloading enabled) */
    if (use_gpu_offload) {
        printf("Test 6: Pointer-based SIMD\n");
        float **a2d = (float**)malloc(N * sizeof(float*));
        float **b2d = (float**)malloc(N * sizeof(float*));
        float **c2d = (float**)malloc(N * sizeof(float*));
        
        for (int i = 0; i < N; i++) {
            a2d[i] = (float*)aligned_alloc(32, M * sizeof(float));
            b2d[i] = (float*)aligned_alloc(32, M * sizeof(float));
            c2d[i] = (float*)aligned_alloc(32, M * sizeof(float));
            
            #pragma omp simd
            for (int j = 0; j < M; j++) {
                a2d[i][j] = (i * M + j) * 0.1f;
                b2d[i][j] = (i * M + j) * 0.2f;
            }
        }
        
        test_pointer_simd(a2d, b2d, c2d, N, M);
        
        float sum6 = 0.0f;
        for (int i = 0; i < N; i++) {
            #pragma omp simd reduction(+:sum6)
            for (int j = 0; j < M; j++) {
                sum6 += c2d[i][j];
            }
            free(a2d[i]);
            free(b2d[i]);
            free(c2d[i]);
        }
        free(a2d);
        free(b2d);
        free(c2d);
        
        checksum += sum6;
        printf("  Checksum 6: %f\n", sum6);
    }
    
    printf("Total checksum: %f\n", checksum);
    
    /* Validation: Compare GPU and CPU results if both paths were tested */
    if (use_gpu_offload) {
        /* Re-run test 1 on CPU for comparison */
        float *c_cpu = (float*)aligned_alloc(32, N * sizeof(float));
        int save_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(a, b, c_cpu, N);
        use_gpu_offload = save_flag;
        
        float diff = 0.0f;
        #pragma omp simd reduction(+:diff)
        for (int i = 0; i < N; i++) {
            diff += fabs(c1[i] - c_cpu[i]);
        }
        printf("Validation diff (GPU vs CPU): %f\n", diff);
        free(c_cpu);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c1);
    free(c2);
    free(c3);
    free(ia);
    free(ib);
    free(ic);
    
    return 0;
}
