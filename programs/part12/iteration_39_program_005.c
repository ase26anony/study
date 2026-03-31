/* test_simt_transformation.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_func(float a, float b, int i) {
    return a * b + i * 0.5f;
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 32)
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
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        private(a, b, c) /* This triggers additional analysis */
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for collapse(2) simd simdlen(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = simd_func(a[idx], b[idx], idx);
            }
        }
    }
}

/* Test 4: Reduction with SIMD in target region */
float test_reduction_simd(float *a, float *b, int n) {
    float sum = 0.0f;
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(tofrom: sum) \
            reduction(+:sum) simdlen(4)
        for (int i = 0; i < n; i++) {
            sum += a[i] * b[i];
        }
    } else {
        #pragma omp simd reduction(+:sum) simdlen(4)
        for (int i = 0; i < n; i++) {
            sum += a[i] * b[i];
        }
    }
    
    return sum;
}

/* Test 5: Linear clause with pointer arithmetic */
void test_linear_simd(float *a, float *b, float *c, int n) {
    float *pa = a;
    float *pb = b;
    float *pc = c;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: pa[0:n], pb[0:n]) map(from: pc[0:n]) \
        simdlen(4) linear(pa, pb, pc: 1) \
        if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        *pc = *pa + *pb;
        pa++;
        pb++;
        pc++;
    }
}

/* Test 6: Mixed directives - simd inside for */
void test_mixed_directives(int *a, int *b, int *c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        #pragma omp simd simdlen(4)
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < n) {
                c[idx] = a[idx] + b[idx] * j;
            }
        }
    }
}

/* Initialize arrays with pattern */
void init_arrays(float *a, float *b, int *ia, int *ib, int n) {
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.1f;
        b[i] = (n - i) * 0.2f;
        ia[i] = i;
        ib[i] = n - i;
    }
}

/* Compute checksum for validation */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(8)
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
    
    /* Allocate arrays with different alignments */
    float *a = (float*)aligned_alloc(32, N * sizeof(float));
    float *b = (float*)aligned_alloc(32, N * sizeof(float));
    float *c1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *c2 = (float*)aligned_alloc(32, N * sizeof(float));
    float *c3 = (float*)aligned_alloc(32, N * M * sizeof(float));
    
    int *ia = (int*)aligned_alloc(64, N * sizeof(int));
    int *ib = (int*)aligned_alloc(64, N * sizeof(int));
    int *ic = (int*)aligned_alloc(64, N * sizeof(int));
    
    if (!a || !b || !c1 || !c2 || !c3 || !ia || !ib || !ic) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, ia, ib, N);
    
    printf("Running SIMT transformation tests...\n");
    
    /* Test 1: Conditional target SIMD */
    printf("\nTest 1: Conditional target SIMD\n");
    test_target_simd(a, b, c1, N);
    float checksum1 = compute_checksum(c1, N);
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(ia, ib, ic, N);
    int int_sum = 0;
    #pragma omp simd reduction(+:int_sum)
    for (int i = 0; i < N; i++) int_sum += ic[i];
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a, b, c3, N, M);
    float checksum3 = compute_checksum(c3, N * M);
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Reduction SIMD */
    printf("\nTest 4: Reduction SIMD\n");
    float reduction_sum = test_reduction_simd(a, b, N);
    printf("Reduction sum: %f\n", reduction_sum);
    
    /* Test 5: Linear clause SIMD */
    printf("\nTest 5: Linear clause SIMD\n");
    test_linear_simd(a, b, c2, N);
    float checksum5 = compute_checksum(c2, N);
    printf("Checksum 5: %f\n", checksum5);
    
    /* Test 6: Mixed directives */
    printf("\nTest 6: Mixed directives\n");
    test_mixed_directives(ia, ib, ic, N/4);
    int_sum = 0;
    #pragma omp simd reduction(+:int_sum)
    for (int i = 0; i < N; i++) int_sum += ic[i];
    printf("Mixed directives sum: %d\n", int_sum);
    
    /* Validation: Compare GPU and CPU paths if both were executed */
    if (use_gpu_offload) {
        /* Re-run test 1 without GPU for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c1_cpu = (float*)aligned_alloc(32, N * sizeof(float));
        test_target_simd(a, b, c1_cpu, N);
        float cpu_checksum = compute_checksum(c1_cpu, N);
        
        printf("\nValidation: GPU checksum = %f, CPU checksum = %f\n", 
               checksum1, cpu_checksum);
        
        /* Simple tolerance check */
        float diff = checksum1 - cpu_checksum;
        if (diff < 0) diff = -diff;
        if (diff < 0.001f) {
            printf("Results match within tolerance\n");
        } else {
            printf("WARNING: Results differ significantly\n");
        }
        
        free(c1_cpu);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3);
    free(ia); free(ib); free(ic);
    
    return 0;
}
