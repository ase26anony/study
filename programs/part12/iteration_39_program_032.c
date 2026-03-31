#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 1024
#define M 32

static int use_gpu_offload = 0;

/* Declare SIMD function for vector addition */
#pragma omp declare simd uniform(a, b) linear(i:1) simdlen(8)
float simd_add(float a, float b, int i) {
    return a + b + (i * 0.001f);
}

/* Test 1: Target teams distribute parallel for simd with conditional offloading */
void test_target_simd(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        if(target: use_gpu_offload) \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(4) safelen(8) aligned(a, b, c: 32) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] + simd_add(a[i], b[i], i);
    }
}

/* Test 2: Parallel for simd with various clauses */
void test_parallel_for_simd(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd \
        simdlen(8) safelen(16) aligned(a, b, c: 64) \
        private(a, b, c) linear(i:1) reduction(+:n)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] - i;
        n += i;  // dummy reduction
    }
}

/* Test 3: Nested SIMD with collapse */
void test_nested_simd(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel
    {
        #pragma omp for simd collapse(2) simdlen(2) safelen(4) \
            aligned(a, b, c: 16) linear(i, j:1)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                c[idx] = a[idx] * 2.5 + b[idx] / 1.5;
            }
        }
    }
}

/* Test 4: SIMD with multiple data types and complex access pattern */
void test_mixed_simd(float *fa, double *da, int *ia, long *la, int n) {
    #pragma omp simd simdlen(4) safelen(8) aligned(fa, da, ia, la: 32) \
        linear(i:1)
    for (int i = 0; i < n; i++) {
        fa[i] = (float)(da[i] * 1.5);
        ia[i] = (int)(fa[i] * 2.0);
        la[i] = (long)(ia[i] * 3L);
    }
}

/* Test 5: Target simd with device-specific clauses */
void test_device_simd(float *a, float *b, float *c, int n) {
    if (use_gpu_offload) {
        #pragma omp target enter data map(to: a[0:n], b[0:n]) map(alloc: c[0:n])
        
        #pragma omp target teams distribute simd \
            device(0) map(always, tofrom: c[0:n]) \
            simdlen(8) num_teams(n/256) thread_limit(64)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] * b[i] + (float)i / n;
        }
        
        #pragma omp target exit data map(from: c[0:n]) map(release: a, b)
    } else {
        #pragma omp simd simdlen(8) aligned(a, b, c: 64)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] * b[i] + (float)i / n;
        }
    }
}

/* Helper function to compute checksum */
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
    
    /* Allocate and initialize arrays */
    float *fa1 = (float*)aligned_alloc(64, N * sizeof(float));
    float *fb1 = (float*)aligned_alloc(64, N * sizeof(float));
    float *fc1 = (float*)aligned_alloc(64, N * sizeof(float));
    
    int *ia = (int*)aligned_alloc(64, N * sizeof(int));
    int *ib = (int*)aligned_alloc(64, N * sizeof(int));
    int *ic = (int*)aligned_alloc(64, N * sizeof(int));
    
    double *da = (double*)aligned_alloc(64, N * M * sizeof(double));
    double *db = (double*)aligned_alloc(64, N * M * sizeof(double));
    double *dc = (double*)aligned_alloc(64, N * M * sizeof(double));
    
    long *la = (long*)aligned_alloc(64, N * sizeof(long));
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N; i++) {
        fa1[i] = (float)i;
        fb1[i] = (float)(N - i);
        fc1[i] = 0.0f;
        ia[i] = i % 100;
        ib[i] = (i * 2) % 100;
        ic[i] = 0;
        la[i] = i * 10L;
    }
    
    #pragma omp parallel for simd collapse(2) simdlen(4)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            da[idx] = (double)(i * M + j);
            db[idx] = (double)((i * M + j) * 2);
            dc[idx] = 0.0;
        }
    }
    
    printf("Starting OpenMP SIMD tests...\n");
    
    /* Test 1: Target SIMD with conditional offloading */
    printf("\nTest 1: Target teams distribute parallel for simd\n");
    test_target_simd(fa1, fb1, fc1, N);
    float checksum1 = compute_checksum(fc1, N);
    printf("Checksum 1: %f\n", checksum1);
    
    /* Test 2: Parallel for SIMD */
    printf("\nTest 2: Parallel for simd\n");
    test_parallel_for_simd(ia, ib, ic, N);
    int sum2 = 0;
    #pragma omp simd reduction(+:sum2)
    for (int i = 0; i < N; i++) sum2 += ic[i];
    printf("Checksum 2: %d\n", sum2);
    
    /* Test 3: Nested SIMD */
    printf("\nTest 3: Nested SIMD with collapse\n");
    test_nested_simd(da, db, dc, N, M);
    double checksum3 = 0.0;
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < N * M; i++) checksum3 += dc[i];
    printf("Checksum 3: %f\n", checksum3);
    
    /* Test 4: Mixed SIMD */
    printf("\nTest 4: Mixed data type SIMD\n");
    test_mixed_simd(fa1, da, ia, la, N);
    float checksum4 = compute_checksum(fa1, N);
    printf("Checksum 4: %f\n", checksum4);
    
    /* Test 5: Device-specific SIMD */
    printf("\nTest 5: Device-specific SIMD\n");
    test_device_simd(fa1, fb1, fc1, N);
    float checksum5 = compute_checksum(fc1, N);
    printf("Checksum 5: %f\n", checksum5);
    
    /* Validation: Compare with reference implementation */
    printf("\nValidation: Computing reference results...\n");
    float *ref = (float*)aligned_alloc(64, N * sizeof(float));
    
    /* Reference computation without OpenMP */
    for (int i = 0; i < N; i++) {
        ref[i] = fa1[i] + fb1[i];
    }
    
    float ref_checksum = compute_checksum(ref, N);
    printf("Reference checksum: %f\n", ref_checksum);
    
    /* Cleanup */
    free(fa1); free(fb1); free(fc1);
    free(ia); free(ib); free(ic);
    free(da); free(db); free(dc);
    free(la); free(ref);
    
    printf("\nAll tests completed.\n");
    return 0;
}
