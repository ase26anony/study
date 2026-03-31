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
            num_teams(4) thread_limit(128) \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) aligned(a, b, c: 32) \
            private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    } else {
        /* Host fallback version */
        #pragma omp simd simdlen(4) safelen(8) aligned(a, b, c: 32) \
                linear(i:1) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
        }
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd simdlen(8) safelen(16) \
            collapse(2) schedule(static) \
            aligned(a, b, c: 64) private(i, j) \
            lastprivate(last_i, last_j)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] * 2 + b[idx] / 3;
        }
    }
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(double *a, double *b, double *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) \
                aligned(a, b, c: 16) linear(i, j:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] * b[idx] - (i + j) * 0.5;
            }
        }
    }
}

/* Test 4: Mixed directives - simd inside for */
void test_mixed_directives(float *a, float *b, float *c, int n) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int block = 0; block < n; block += 64) {
            int end = (block + 64 < n) ? block + 64 : n;
            #pragma omp simd simdlen(4) aligned(a, b, c: 16)
            for (int i = block; i < end; i++) {
                c[i] = (a[i] - b[i]) * (a[i] + b[i]);
            }
        }
    }
}

/* Test 5: Target with dynamic data mapping */
void test_dynamic_mapping(float **a_ptr, float **b_ptr, float **c_ptr, int n) {
    float *a = *a_ptr;
    float *b = *b_ptr;
    float *c = *c_ptr;
    
    if (use_gpu_offload) {
        #pragma omp target enter data map(to: a[0:n], b[0:n]) map(alloc: c[0:n])
        
        #pragma omp target teams distribute parallel for simd \
                simdlen(4) map(always, tofrom: c[0:n])
        for (int i = 0; i < n; i++) {
            c[i] = sinf(a[i]) * cosf(b[i]);
        }
        
        #pragma omp target exit data map(from: c[0:n]) map(release: a[0:n], b[0:n])
    } else {
        #pragma omp simd simdlen(4)
        for (int i = 0; i < n; i++) {
            c[i] = sinf(a[i]) * cosf(b[i]);
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

int main(int argc, char *argv[]) {
    /* Parse command line argument for GPU offloading */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    const int total_size = N * M;
    
    /* Allocate and initialize arrays with patterned data */
    float *a_f = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *b_f = (float*)aligned_alloc(32, total_size * sizeof(float));
    float *c_f = (float*)aligned_alloc(32, total_size * sizeof(float));
    
    int *a_i = (int*)aligned_alloc(64, total_size * sizeof(int));
    int *b_i = (int*)aligned_alloc(64, total_size * sizeof(int));
    int *c_i = (int*)aligned_alloc(64, total_size * sizeof(int));
    
    double *a_d = (double*)aligned_alloc(16, total_size * sizeof(double));
    double *b_d = (double*)aligned_alloc(16, total_size * sizeof(double));
    double *c_d = (double*)aligned_alloc(16, total_size * sizeof(double));
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        a_f[i] = i * 0.1f;
        b_f[i] = (total_size - i) * 0.1f;
        c_f[i] = 0.0f;
        
        a_i[i] = i;
        b_i[i] = total_size - i;
        c_i[i] = 0;
        
        a_d[i] = i * 0.01;
        b_d[i] = (total_size - i) * 0.01;
        c_d[i] = 0.0;
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Execute test functions */
    test_target_simd(a_f, b_f, c_f, total_size);
    float checksum1 = compute_checksum(c_f, total_size);
    printf("Test 1 checksum: %f\n", checksum1);
    
    test_parallel_for_simd(a_i, b_i, c_i, N);
    float checksum2 = 0;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < total_size; i++) {
        checksum2 += c_i[i];
    }
    printf("Test 2 checksum: %f\n", checksum2);
    
    test_nested_simd(a_d, b_d, c_d, N);
    double checksum3 = 0;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < total_size; i++) {
        checksum3 += c_d[i];
    }
    printf("Test 3 checksum: %f\n", (float)checksum3);
    
    /* Reset and test mixed directives */
    memset(c_f, 0, total_size * sizeof(float));
    test_mixed_directives(a_f, b_f, c_f, total_size);
    float checksum4 = compute_checksum(c_f, total_size);
    printf("Test 4 checksum: %f\n", checksum4);
    
    /* Test with dynamic mapping */
    float *a_dyn = (float*)malloc(total_size * sizeof(float));
    float *b_dyn = (float*)malloc(total_size * sizeof(float));
    float *c_dyn = (float*)malloc(total_size * sizeof(float));
    
    #pragma omp simd
    for (int i = 0; i < total_size; i++) {
        a_dyn[i] = sinf(i * 0.01f);
        b_dyn[i] = cosf(i * 0.01f);
        c_dyn[i] = 0.0f;
    }
    
    test_dynamic_mapping(&a_dyn, &b_dyn, &c_dyn, total_size);
    float checksum5 = compute_checksum(c_dyn, total_size);
    printf("Test 5 checksum: %f\n", checksum5);
    
    /* Validation: Run both GPU and CPU paths if GPU was used */
    if (use_gpu_offload) {
        printf("\nValidating GPU vs CPU results...\n");
        
        /* Run CPU-only version for comparison */
        use_gpu_offload = 0;
        float *c_cpu = (float*)aligned_alloc(32, total_size * sizeof(float));
        memset(c_cpu, 0, total_size * sizeof(float));
        
        #pragma omp simd simdlen(4)
        for (int i = 0; i < total_size; i++) {
            c_cpu[i] = a_f[i] + b_f[i] + simd_add(a_f[i], b_f[i], i);
        }
        
        float cpu_checksum = compute_checksum(c_cpu, total_size);
        printf("CPU checksum: %f, GPU-initial checksum: %f\n", 
               cpu_checksum, checksum1);
        
        free(c_cpu);
    }
    
    /* Cleanup */
    free(a_f); free(b_f); free(c_f);
    free(a_i); free(b_i); free(c_i);
    free(a_d); free(b_d); free(c_d);
    free(a_dyn); free(b_dyn); free(c_dyn);
    
    printf("All tests completed.\n");
    return 0;
}
