#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

/* Declare SIMD function for vector addition */
#pragma omp declare simd uniform(a, b) linear(i:1) aligned(a, b:16)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target SIMD with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        /* This should trigger SIMT transformation when compiled with -foffload */
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 16)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        /* Host fallback */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 16)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test 2: Parallel for SIMD with various clauses */
void test_parallel_for_simd(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd reduction(+:sum) \
        simdlen(8) safelen(16) aligned(a, b, c: 32) \
        private(i) schedule(static, 64)
    for (int i = 0; i < n; i++) {
        c[i] = simd_add(a[i], b[i], i);
        sum += c[i];
    }
    
    printf("Reduction sum: %.2f\n", sum);
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(float *a, float *b, float *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4) \
            linear(i, j:1) aligned(a, b, c: 16)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * b[idx] - (float)(i + j);
            }
        }
    }
}

/* Test 4: Teams distribute with multiple levels */
void test_teams_distribute_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
        num_teams(8) thread_limit(128) \
        simdlen(4) safelen(8) collapse(1) \
        private(i) firstprivate(n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * 2.0f - b[i] / 2.0f;
    }
}

/* Test 5: Mixed directives - SIMD inside parallel region */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i += 64) {
            int end = (i + 64 < n) ? i + 64 : n;
            
            /* This inner loop should be SIMD vectorized */
            #pragma omp simd simdlen(8) aligned(a, b, c: 16)
            for (int j = i; j < end; j++) {
                c[j] = (a[j] + b[j]) * (a[j] - b[j]);
            }
        }
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

int main(int argc, char **argv) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    /* Allocate and initialize arrays with dynamic allocation */
    int total_size = N * M;
    float *a = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *b = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *c1 = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *c2 = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *c3 = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *c4 = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *c5 = (float*)aligned_alloc(32, total_size * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < total_size; i++) {
        a[i] = (float)i;
        b[i] = (float)(total_size - i);
        c1[i] = c2[i] = c3[i] = c4[i] = c5[i] = 0.0f;
    }
    
    printf("Array size: %d elements\n", total_size);
    
    /* Execute test functions with different SIMD constructs */
    printf("\n=== Test 1: Target SIMD ===\n");
    test_target_simd(a, b, c1, total_size);
    float sum1 = compute_checksum(c1, total_size);
    printf("Checksum 1: %.2f\n", sum1);
    
    printf("\n=== Test 2: Parallel for SIMD ===\n");
    test_parallel_for_simd(a, b, c2, total_size);
    float sum2 = compute_checksum(c2, total_size);
    printf("Checksum 2: %.2f\n", sum2);
    
    printf("\n=== Test 3: Nested SIMD ===\n");
    test_nested_simd(a, b, c3, N, M);
    float sum3 = compute_checksum(c3, total_size);
    printf("Checksum 3: %.2f\n", sum3);
    
    printf("\n=== Test 4: Teams distribute SIMD ===\n");
    test_teams_distribute_simd(a, b, c4, total_size);
    float sum4 = compute_checksum(c4, total_size);
    printf("Checksum 4: %.2f\n", sum4);
    
    printf("\n=== Test 5: Mixed directives ===\n");
    test_mixed_directives(a, b, c5, total_size);
    float sum5 = compute_checksum(c5, total_size);
    printf("Checksum 5: %.2f\n", sum5);
    
    /* Validation: Compare results if both paths were executed */
    if (use_gpu_offload) {
        /* In real scenario, we'd compare GPU vs CPU results */
        printf("\n=== Validation ===\n");
        
        /* Recompute test 1 on host for comparison */
        float *c_host = (float*)aligned_alloc(32, total_size * sizeof(float));
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(a, b, c_host, total_size);
        use_gpu_offload = saved_flag;
        
        float diff = 0.0f;
        #pragma omp simd reduction(+:diff) simdlen(8)
        for (int i = 0; i < total_size; i++) {
            diff += fabsf(c1[i] - c_host[i]);
        }
        
        if (diff < 0.001f) {
            printf("GPU and CPU results match (diff=%.6f)\n", diff);
        } else {
            printf("WARNING: GPU and CPU results differ (diff=%.6f)\n", diff);
        }
        
        free(c_host);
    }
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(c5);
    
    return 0;
}
