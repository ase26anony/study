#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Flow dependence with distance 1, mixed with anti-dependence */
    int temp;
    for (int i = 1; i < n; i++) {
        temp = a[i - 1];  /* Read a[i-1] */
        a[i] = temp + b[i];  /* Write a[i] - anti-dependence with next iteration's read */
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n) {
    /* Contains flow, anti, and output dependences */
    int t1, t2;
    for (int i = 1; i < n; i++) {
        t1 = a[i - 1];           /* Flow: read a[i-1] from previous iteration */
        a[i] = t1 + b[i];        /* Write a[i] - creates anti for next iteration */
        t2 = c[i];               /* Read c[i] */
        c[i - 1] = t2 * 2;       /* Write c[i-1] - output dependence with previous iteration */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict qualifier - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence due to p self-overlap */
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
static void test_volatile_dependence(volatile int *v, int n) {
    /* Volatile ensures memory accesses aren't reordered/eliminated */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
    
    /* Additional volatile anti-dependence */
    volatile int temp;
    for (int i = 0; i < n - 1; i++) {
        temp = v[i];     /* Read */
        v[i] = v[i + 1]; /* Write - anti-dependence */
    }
}

/* Test 6: Different data types for different edge data_type fields */
static void test_mixed_data_types(float *fa, double *da, int *ia, int n) {
    /* Float array with flow dependence */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;
    }
    
    /* Double array with anti-dependence */
    double temp;
    for (int i = 0; i < n - 1; i++) {
        temp = da[i];          /* Read */
        da[i] = da[i + 1];     /* Write - anti-dependence */
    }
    
    /* Integer array with output dependence */
    for (int i = 1; i < n; i++) {
        ia[i] = ia[i] + 1;     /* Read-modify-write creates output dependence */
    }
}

/* Test 7: Complex index calculations to hinder analysis */
static void test_complex_indices(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 3 + 7) % n;
        int idx2 = (i * 5 + 11) % n;
        arr[idx1] = arr[idx2] + i;
    }
}

int main() {
    /* Initialize with deterministic but non-trivial values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *p = (int*)malloc(SIZE * sizeof(int));
    int *q = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    int *ia = (int*)malloc(SIZE * sizeof(int));
    int (*arr2d)[N] = (int(*)[N])malloc(M * N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        v[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        ia[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure computations aren't optimized away */
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE);
    test_mixed_dependences(a, b, c, SIZE);
    test_pointer_aliasing(p, q, SIZE);
    test_nested_loops(arr2d);
    test_volatile_dependence(v, SIZE);
    test_mixed_data_types(fa, da, ia, SIZE);
    test_complex_indices(a, SIZE);
    
    /* Compute checksum from all arrays */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + p[i] + q[i] + v[i] + (int)fa[i] + (int)da[i] + ia[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(p); free(q); free((void*)v);
    free(fa); free(da); free(ia); free(arr2d);
    
    return 0;
}
