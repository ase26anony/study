#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 64

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, int *sum) {
    volatile int *va = (volatile int *)a;
    volatile int *vb = (volatile int *)b;
    
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        va[i + 2] = va[i] * vb[i];
    }
    
    /* Anti-dependence (WAR) */
    int temp = 0;
    for (int i = 1; i < n; i++) {
        temp = va[i - 1];  /* Read */
        va[i] = temp + i;  /* Write - anti-dependent on previous read */
    }
    
    *sum += va[n-1];
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(double *arr, int n, double *sum) {
    volatile double *varr = (volatile double *)arr;
    double t = 0.0;
    
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        t = varr[i - 1];               /* Flow: read i-1, write t */
        varr[i] = t + (double)i;       /* Anti: read t, write arr[i] */
        varr[i - 1] = varr[i] * 0.5;   /* Output: write arr[i-1] again */
    }
    
    *sum += varr[n-1];
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n, int *sum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
    
    for (int i = 0; i < n; i++) {
        *sum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_dependence(int arr[M][N], int *sum) {
    volatile int (*varr)[N] = (volatile int (*)[N])arr;
    
    /* Inner loop has distance-2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            varr[i][j] = varr[i][j-2] + varr[i][j-1];
        }
    }
    
    for (int i = 0; i < M; i++) {
        *sum += varr[i][N-1];
    }
}

/* Test 5: Complex index calculations with volatile */
static void test_complex_indices(float *data, int n, float *fsum) {
    volatile float *vdata = (volatile float *)data;
    volatile int indices[N];
    
    /* Initialize volatile indices */
    for (int i = 0; i < n; i++) {
        indices[i] = (i * 13 + 7) % n;
    }
    
    /* Loop with indirect addressing causing various dependences */
    for (int i = 1; i < n; i++) {
        int idx1 = indices[i];
        int idx2 = indices[i-1];
        
        /* Multiple potential dependences */
        float temp = vdata[idx2];          /* Read */
        vdata[idx1] = temp + vdata[i];     /* Write - anti on temp, flow on vdata[i] */
        vdata[i] = vdata[idx1] * 0.5f;     /* Output on vdata[i] */
    }
    
    for (int i = 0; i < n; i++) {
        *fsum += vdata[i];
    }
}

/* Test 6: Different data types to affect data_type field */
static void test_mixed_data_types(int *results) {
    volatile int vi[N];
    volatile float vf[N];
    volatile double vd[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        vi[i] = i;
        vf[i] = i * 1.5f;
        vd[i] = i * 2.5;
    }
    
    /* Integer loop with distance 3 */
    for (int i = 0; i < N - 3; i++) {
        vi[i + 3] = vi[i] + vi[i + 1];
    }
    
    /* Float loop with distance 1 */
    for (int i = 1; i < N; i++) {
        vf[i] = vf[i - 1] * 1.1f;
    }
    
    /* Double loop with mixed dependences */
    for (int i = 2; i < N; i++) {
        double temp = vd[i - 2];
        vd[i - 1] = temp + vd[i];  /* Anti on temp, flow on vd[i] */
        vd[i] = vd[i - 1] * 0.9;   /* Flow on vd[i-1] */
    }
    
    results[0] = vi[N-1];
    results[1] = (int)vf[N-1];
    results[2] = (int)vd[N-1];
}

int main(void) {
    /* Initialize with deterministic values */
    srand(42);
    
    int a[N], b[N];
    double darr[N];
    float farr[N];
    int matrix[M][N];
    int results[3];
    
    volatile int checksum = 0;
    volatile float fsum = 0.0f;
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        darr[i] = (double)(rand() % 100) / 3.0;
        farr[i] = (float)(rand() % 100) / 2.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    
    /* Execute tests to trigger DDG edge creation */
    test_flow_dependence(a, b, N, &sum);
    checksum += sum;
    
    test_mixed_dependences(darr, N, &darr[0]);  /* Reuse darr[0] as temp */
    checksum += (int)darr[N-1];
    
    test_pointer_aliasing(a, b, N, &sum);
    checksum += sum;
    
    test_nested_dependence(matrix, &sum);
    checksum += sum;
    
    test_complex_indices(farr, N, &fsum);
    checksum += (int)fsum;
    
    test_mixed_data_types(results);
    checksum += results[0] + results[1] + results[2];
    
    /* Use results to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
