#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

/* Global flag to control GPU offloading */
static int use_gpu_offload = 0;

/* Function with declare simd pragma */
#pragma omp declare simd uniform(a, b) linear(i:1)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target teams distribute parallel for simd with conditional execution */
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(64) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
            sum += c[i];
        }
        printf("GPU offload sum: %f\n", sum);
    } else {
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
            sum += c[i];
        }
        printf("Host SIMD sum: %f\n", sum);
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        private(i) schedule(static, 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + i;
    }
    
    /* Nested SIMD region */
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(4)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < n) {
                    c[idx] += simd_add(a[idx], b[idx], idx);
                }
            }
        }
    }
}

/* Test 3: Complex nested SIMD with collapse and linear clauses */
void test_nested_simd(double *a, double *b, double *c, int n) {
    int block_size = 16;
    
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
        simdlen(2) safelen(4) \
        linear(i, j:1) private(i, j, idx)
    for (int i = 0; i < n; i += block_size) {
        for (int j = 0; j < block_size && (i + j) < n; j++) {
            int idx = i + j;
            c[idx] = a[idx] * 2.5 + b[idx] * 1.5;
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i += 64) {
            int end = (i + 64 < n) ? i + 64 : n;
            
            #pragma omp simd simdlen(8) aligned(a, b, c: 32)
            for (int j = i; j < end; j++) {
                c[j] = (a[j] - b[j]) * (a[j] + b[j]);
            }
        }
    }
}

/* Test 5: Dynamic pointer-based access with SIMD */
void test_pointer_simd(float **pa, float **pb, float **pc, int n) {
    float *a = *pa;
    float *b = *pb;
    float *c = *pc;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) is_device_ptr(a, b, c) if(target: use_gpu_offload)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] / (b[i] + 1.0f);
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

int main(int argc, char *argv[]) {
    /* Parse command line argument */
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
    
    int *ia = (int*)malloc(N * sizeof(int));
    int *ib = (int*)malloc(N * sizeof(int));
    int *ic = (int*)malloc(N * sizeof(int));
    
    double *da = (double*)malloc(N * sizeof(double));
    double *db = (double*)malloc(N * sizeof(double));
    double *dc = (double*)malloc(N * sizeof(double));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = (N - i) * 1.0f;
        c1[i] = c2[i] = c3[i] = 0.0f;
        
        ia[i] = i;
        ib[i] = N - i;
        ic[i] = 0;
        
        da[i] = i * 0.5;
        db[i] = (N - i) * 0.5;
        dc[i] = 0.0;
    }
    
    printf("Running OpenMP SIMD tests...\n");
    
    /* Test 1: Conditional target SIMD */
    printf("\n=== Test 1: Conditional Target SIMD ===\n");
    test_target_simd(a, b, c1, N);
    
    /* Test 2: Parallel for SIMD */
    printf("\n=== Test 2: Parallel For SIMD ===\n");
    test_parallel_for_simd(ia, ib, ic, N);
    
    /* Compute checksum for Test 2 */
    int sum_int = 0;
    #pragma omp simd reduction(+:sum_int)
    for (int i = 0; i < N; i++) {
        sum_int += ic[i];
    }
    printf("Integer array checksum: %d\n", sum_int);
    
    /* Test 3: Nested SIMD with collapse */
    printf("\n=== Test 3: Nested SIMD with Collapse ===\n");
    test_nested_simd(da, db, dc, N);
    
    /* Test 4: Mixed directives */
    printf("\n=== Test 4: Mixed Directives ===\n");
    test_mixed_directives(a, b, c2, N);
    
    /* Test 5: Pointer-based SIMD */
    printf("\n=== Test 5: Pointer-based SIMD ===\n");
    float *pa = a, *pb = b, *pc = c3;
    test_pointer_simd(&pa, &pb, &pc, N);
    
    /* Validation */
    printf("\n=== Validation ===\n");
    
    /* Compare results from different execution paths if available */
    if (use_gpu_offload) {
        /* Run host version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_host = (float*)aligned_alloc(64, N * sizeof(float));
        memset(c_host, 0, N * sizeof(float));
        
        #pragma omp simd simdlen(4)
        for (int i = 0; i < N; i++) {
            c_host[i] = a[i] + b[i];
        }
        
        int errors = validate_results(c1, c_host, N, 0.001f);
        printf("Validation errors between GPU and host: %d\n", errors);
        
        free(c_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Final checksum of all float results */
    float total_sum = 0.0f;
    #pragma omp simd reduction(+:total_sum) simdlen(8)
    for (int i = 0; i < N; i++) {
        total_sum += c1[i] + c2[i] + c3[i];
    }
    printf("Total checksum of all float arrays: %f\n", total_sum);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    
    return 0;
}
