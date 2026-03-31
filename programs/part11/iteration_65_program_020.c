#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define M 64
#define N 128

/* Test 1: Loop-carried flow dependence with distance 2 */
__attribute__((always_inline))
static inline void test_flow_distance(int *a, int *b, int n, volatile int *checksum) {
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];  /* Flow dependence, distance 2 */
        *checksum += a[i + 2];
    }
}

/* Test 2: Multiple dependence types in one loop */
__attribute__((always_inline))
static inline void test_mixed_deps(int *a, int *b, int *c, int n, volatile int *checksum) {
    int t;
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow dependence: RAW on a[i-1] */
        a[i] = t + b[i];        /* Anti-dependence: WAR on t */
        c[i] = c[i - 1] * 2;    /* Flow dependence: RAW on c[i-1] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Output dependence (WAW) with volatile */
static void test_output_dep(volatile int *v, int n, volatile int *checksum) {
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;    /* Flow dependence + potential WAW */
        v[i] = v[i] * 2;        /* Output dependence on v[i] */
        *checksum += v[i];
    }
}

/* Test 4: Pointer aliasing with restrict and non-restrict */
static void test_aliasing(int * restrict p, int *q, int n, volatile int *checksum) {
    /* No restrict here - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
        *checksum += p[i];
    }
}

/* Test 5: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested(int arr[M][N], volatile int *checksum) {
    #pragma GCC unroll 4
    for (int i = 0; i < M; i++) {
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;  /* Flow dependence, distance 2 */
            *checksum += arr[i][j];
        }
    }
}

/* Test 6: Different data types for varied data_type field */
static void test_mixed_types(float *fa, double *da, int *ia, int n, volatile int *checksum) {
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;      /* float flow dependence */
        da[i] = da[i - 1] / 2.0;       /* double flow dependence */
        ia[i] = ia[i - 1] + 1;         /* int flow dependence */
        *checksum += (int)(fa[i] + da[i] + ia[i]);
    }
}

/* Test 7: Complex index calculations with modulo */
static void test_complex_index(int *a, int *b, int n, volatile int *checksum) {
    for (int i = 0; i < n; i++) {
        int idx = (i * 13 + 7) % n;
        a[idx] = b[i] + a[(idx + 1) % n];  /* Multiple potential dependences */
        *checksum += a[idx];
    }
}

int main(void) {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    int *ia = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    int (*arr)[N] = (int(*)[N])malloc(M * N * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        ia[i] = rand() % 100;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_distance(a, b, SIZE, &checksum);
    test_mixed_deps(a, b, c, SIZE, &checksum);
    test_output_dep(v, SIZE, &checksum);
    test_aliasing(a, b, SIZE, &checksum);
    test_nested(arr, &checksum);
    test_mixed_types(fa, da, ia, SIZE, &checksum);
    test_complex_index(a, b, SIZE, &checksum);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(fa); free(da); free(ia); 
    free((void*)v); free(arr);
    
    return 0;
}
