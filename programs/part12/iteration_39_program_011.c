#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

// Global flag to control GPU offloading
static int use_gpu_offload = 0;

// Function with declare simd for vectorization
#pragma omp declare simd uniform(a, b) linear(i:1)
float compute_element(float a, float b, int i) {
    return a * b + (i % 10) * 0.1f;
}

// Test 1: Target SIMD with conditional offloading
void test_target_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        // This should trigger SIMT transformation when compiled with -foffload
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            simdlen(4) safelen(8) private(i) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        // Host-only version
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
                c[idx] = compute_element(a[idx], b[idx], idx);
            }
        }
    }
}

// Test 4: Teams distribute with SIMD - complex nesting
void test_teams_distribute_simd(double *a, double *b, double *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:n], b[0:n]) map(tofrom: c[0:n]) \
            num_teams(4) thread_limit(128) simdlen(2)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] / (b[i] + 1.0);
        }
    } else {
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] / (b[i] + 1.0);
        }
    }
}

// Test 5: SIMD with reduction and private variables
void test_simd_reduction(float *arr, int n, float *result) {
    float sum = 0.0f;
    float min_val = 1e9f;
    float max_val = -1e9f;
    
    #pragma omp simd reduction(+:sum) reduction(min:min_val) \
        reduction(max:max_val) simdlen(4) safelen(32)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
    }
    
    result[0] = sum;
    result[1] = min_val;
    result[2] = max_val;
}

int main(int argc, char **argv) {
    // Parse command line argument for GPU offloading
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-gpu") == 0) {
            use_gpu_offload = 1;
            printf("GPU offloading enabled\n");
        }
    }
    
    // Allocate and initialize arrays with dynamic allocation
    float *fa = (float*)malloc(N * sizeof(float));
    float *fb = (float*)malloc(N * sizeof(float));
    float *fc = (float*)malloc(N * sizeof(float));
    float *fc_host = (float*)malloc(N * sizeof(float));
    
    int *ia = (int*)malloc(N * sizeof(int));
    int *ib = (int*)malloc(N * sizeof(int));
    int *ic = (int*)malloc(N * sizeof(int));
    
    double *da = (double*)malloc(N * sizeof(double));
    double *db = (double*)malloc(N * sizeof(double));
    double *dc = (double*)malloc(N * sizeof(double));
    
    // Initialize with patterned data
    for (int i = 0; i < N; i++) {
        fa[i] = (float)i;
        fb[i] = (float)(N - i);
        fc[i] = 0.0f;
        fc_host[i] = 0.0f;
        
        ia[i] = i % 100;
        ib[i] = (i * 3) % 100;
        ic[i] = 0;
        
        da[i] = (double)i * 0.5;
        db[i] = (double)(i + 1) * 0.3;
        dc[i] = 0.0;
    }
    
    float checksum = 0.0f;
    
    // Test 1: Target SIMD with conditional execution
    printf("Test 1: Target SIMD\n");
    test_target_simd(fa, fb, fc, N);
    
    // Compute checksum to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        checksum += fc[i];
    }
    printf("Checksum 1: %f\n", checksum);
    
    // Test 2: Parallel for SIMD
    printf("\nTest 2: Parallel for SIMD\n");
    test_parallel_for_simd(ia, ib, ic, N);
    
    int sum_int = 0;
    for (int i = 0; i < N; i++) {
        sum_int += ic[i];
    }
    printf("Integer sum: %d\n", sum_int);
    
    // Test 3: Nested SIMD
    printf("\nTest 3: Nested SIMD\n");
    test_nested_simd(fa, fb, fc_host, N/2, 2);
    
    checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += fc_host[i];
    }
    printf("Checksum 3: %f\n", checksum);
    
    // Test 4: Teams distribute SIMD
    printf("\nTest 4: Teams distribute SIMD\n");
    test_teams_distribute_simd(da, db, dc, N);
    
    double sum_double = 0.0;
    for (int i = 0; i < N; i++) {
        sum_double += dc[i];
    }
    printf("Double sum: %f\n", sum_double);
    
    // Test 5: SIMD with reduction
    printf("\nTest 5: SIMD with reduction\n");
    float reduction_result[3];
    test_simd_reduction(fa, N, reduction_result);
    printf("Reduction - Sum: %f, Min: %f, Max: %f\n", 
           reduction_result[0], reduction_result[1], reduction_result[2]);
    
    // Validation: Compare GPU and CPU results if both paths were tested
    if (use_gpu_offload) {
        // Re-run test 1 on host for comparison
        float *fc_cpu = (float*)malloc(N * sizeof(float));
        int save_flag = use_gpu_offload;
        use_gpu_offload = 0;
        test_target_simd(fa, fb, fc_cpu, N);
        use_gpu_offload = save_flag;
        
        // Compare results
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(fc[i] - fc_cpu[i]) > 1e-5) {
                errors++;
                if (errors < 5) {
                    printf("Mismatch at %d: GPU=%f, CPU=%f\n", 
                           i, fc[i], fc_cpu[i]);
                }
            }
        }
        printf("Validation: %d errors in %d elements\n", errors, N);
        free(fc_cpu);
    }
    
    // Cleanup
    free(fa);
    free(fb);
    free(fc);
    free(fc_host);
    free(ia);
    free(ib);
    free(ic);
    free(da);
    free(db);
    free(dc);
    
    return 0;
}
