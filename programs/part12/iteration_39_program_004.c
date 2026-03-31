#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 64

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
            simdlen(4) safelen(8) private(n) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        private(n) linear(i:1) reduction(+:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] - i;
        n += i;  /* reduction variable */
    }
}

/* Test 3: Nested loops with collapse clause */
void test_nested_simd(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) safelen(4) \
            aligned(a, b, c: 16) private(i, j)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.5 + b[idx] / 3.0;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i += 4) {
            #pragma omp simd simdlen(4) aligned(a, b, c: 16)
            for (int j = 0; j < 4 && (i + j) < n; j++) {
                int idx = i + j;
                c[idx] = a[idx] - b[idx] * 0.5f;
            }
        }
    }
}

/* Test 5: Dynamic allocation with pointer-based access */
void test_dynamic_allocation(int size) {
    int *x = (int *)malloc(size * sizeof(int));
    int *y = (int *)malloc(size * sizeof(int));
    int *z = (int *)malloc(size * sizeof(int));
    
    if (!x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize arrays */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        x[i] = i;
        y[i] = size - i;
    }
    
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: x[0:size], y[0:size]) map(from: z[0:size]) \
            simdlen(8) safelen(16)
        for (int i = 0; i < size; i++) {
            z[i] = x[i] + y[i] * 2;
        }
    } else {
        #pragma omp simd simdlen(8) safelen(16)
        for (int i = 0; i < size; i++) {
            z[i] = x[i] + y[i] * 2;
        }
    }
    
    /* Compute checksum */
    long long checksum = 0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < size; i++) {
        checksum += z[i];
    }
    printf("Dynamic allocation checksum: %lld\n", checksum);
    
    free(x);
    free(y);
    free(z);
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
    
    double *a_d = (double *)aligned_alloc(16, N * M * sizeof(double));
    double *b_d = (double *)aligned_alloc(16, N * M * sizeof(double));
    double *c_d = (double *)aligned_alloc(16, N * M * sizeof(double));
    
    if (!a_f || !b_f || !c_f || !a_i || !b_i || !c_i || !a_d || !b_d || !c_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with patterns */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (N - i) * 0.2f;
        a_i[i] = i;
        b_i[i] = N - i;
    }
    
    #pragma omp parallel for simd collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            a_d[idx] = i * 0.01 + j * 0.001;
            b_d[idx] = (N - i) * 0.02 + (M - j) * 0.002;
        }
    }
    
    /* Run test functions */
    printf("Running test_target_simd...\n");
    test_target_simd(a_f, b_f, c_f, N);
    
    /* Compute checksum for validation */
    float checksum_f = 0.0f;
    #pragma omp simd reduction(+:checksum_f)
    for (int i = 0; i < N; i++) {
        checksum_f += c_f[i];
    }
    printf("Target SIMD checksum: %f\n", checksum_f);
    
    printf("Running test_parallel_for_simd...\n");
    int n_local = 0;  /* For reduction */
    test_parallel_for_simd(a_i, b_i, c_i, N);
    
    printf("Running test_nested_simd...\n");
    test_nested_simd(a_d, b_d, c_d, N, M);
    
    printf("Running test_mixed_directives...\n");
    test_mixed_directives(a_f, b_f, c_f, N);
    
    printf("Running test_dynamic_allocation...\n");
    test_dynamic_allocation(512);
    
    /* Compare results if both GPU and CPU paths were tested */
    if (use_gpu_offload) {
        /* Re-run host-only version for comparison */
        int saved_flag = use_gpu_offload;
        use_gpu_offload = 0;
        
        float *c_f_host = (float *)aligned_alloc(32, N * sizeof(float));
        test_target_simd(a_f, b_f, c_f_host, N);
        
        /* Compare results */
        int errors = 0;
        #pragma omp parallel for simd reduction(+:errors)
        for (int i = 0; i < N; i++) {
            if (fabs(c_f[i] - c_f_host[i]) > 0.001f) {
                errors++;
            }
        }
        
        if (errors == 0) {
            printf("GPU and CPU results match perfectly\n");
        } else {
            printf("Found %d differences between GPU and CPU results\n", errors);
        }
        
        free(c_f_host);
        use_gpu_offload = saved_flag;
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    
    return 0;
}
