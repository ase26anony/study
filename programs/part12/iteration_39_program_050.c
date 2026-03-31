/* test_simt_transformation.c
 * Designed to trigger SIMT transformation in omp-low.cc lines 2941-2975
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower test_simt_transformation.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

/* Global flag to control offloading at runtime */
static int use_gpu_offload = 0;

/* Function with declare simd for vectorization */
#pragma omp declare simd uniform(a, b) linear(i:1) notinbranch
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test 2: Parallel for simd with reduction and private variables */
void test_parallel_for_simd(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd reduction(+:sum) \
        simdlen(8) safelen(16) private(a, b, c) \
        collapse(2) schedule(static)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            c[idx] = simd_add(a[idx], b[idx], idx);
            sum += c[idx];
        }
    }
    
    printf("Reduction sum: %f\n", sum);
}

/* Test 3: Nested SIMD with complex clauses */
void test_nested_simd(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) \
            linear(i:1) linear(j:1) \
            aligned(a, b, c: 64)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < n/M; j++) {
                int idx = i * (n/M) + j;
                c[idx] = a[idx] * b[idx] - (i + j) * 0.5f;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        #pragma omp simd simdlen(2)
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < n) {
                c[idx] = a[idx] - b[idx];
            }
        }
    }
}

/* Test 5: SIMD with dynamic data and pointer arithmetic */
void test_dynamic_simd(float **a_ptr, float **b_ptr, float **c_ptr, int n) {
    float *a = *a_ptr;
    float *b = *b_ptr;
    float *c = *c_ptr;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) if(use_gpu_offload)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * 2.0f + b[i] * 3.0f;
    }
}

/* Validation function */
int validate_results(float *c1, float *c2, int n, float tolerance) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(c1[i] - c2[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: %f != %f\n", i, c1[i], c2[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char **argv) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c1 = (float*)aligned_alloc(64, N * sizeof(float));
    float *c2 = (float*)aligned_alloc(64, N * sizeof(float));
    float *c3 = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c1[i] = c2[i] = c3[i] = 0.0f;
    }
    
    printf("Running SIMT transformation tests...\n");
    
    /* Test 1: Target SIMD with conditional execution */
    printf("\nTest 1: Target SIMD\n");
    test_target_simd(a, b, c1, N);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum1 = 0.0f;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < N; i++) {
        checksum1 += c1[i];
    }
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD with reduction */
    printf("\nTest 2: Parallel for SIMD with reduction\n");
    test_parallel_for_simd(a, b, c2, N);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(a, b, c3, N);
    
    /* Test 4: Mixed directives */
    printf("\nTest 4: Mixed directives\n");
    float *c4 = (float*)aligned_alloc(64, N * sizeof(float));
    test_mixed_directives(a, b, c4, N/4);
    
    /* Test 5: Dynamic SIMD with pointers */
    printf("\nTest 5: Dynamic SIMD\n");
    float *c5 = (float*)aligned_alloc(64, N * sizeof(float));
    float **a_ptr = &a;
    float **b_ptr = &b;
    float **c_ptr = &c5;
    test_dynamic_simd(a_ptr, b_ptr, c_ptr, N);
    
    /* Validation */
    printf("\nValidation:\n");
    int errors = validate_results(c1, c2, N, 0.001f);
    if (errors == 0) {
        printf("All tests passed!\n");
    } else {
        printf("Found %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c1);
    free(c2);
    free(c3);
    free(c4);
    free(c5);
    
    return 0;
}
