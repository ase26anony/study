#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 256
#define INNER_SIZE 128
#define M 64
#define N 128

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test1_flow_distance(int *a, int *b, int n, volatile int *checksum) {
    /* Flow dependence distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
        *checksum += a[i + 2];
    }
    
    /* Flow dependence distance 3 with float */
    volatile float *vf = (volatile float *)a;
    for (int i = 0; i < n - 3; i++) {
        vf[i + 3] = vf[i] + 1.5f;
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test2_mixed_deps(int *a, int *b, int *c, int n, volatile int *checksum) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Flow: read a[i-1] */
        a[i] = t + b[i];            /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;        /* Flow: read c[i-1], Output: write c[i] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with restrict and non-restrict */
static void test3_aliasing(int *restrict p, int *restrict q, int *r, int n) {
    /* p and q are restrict, but r may alias with anything */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + r[(i * 7) % n];  /* Potential flow if r aliases p */
    }
}

static void test3_no_restrict(int *p, int *q, int n) {
    /* No restrict - compiler must assume all pointers may alias */
    for (int i = 1; i < n; i++) {
        p[i] = p[i - 1] + q[i];        /* Clear flow dependence */
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test4_nested(int arr[M][N], volatile int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory for forced dependences */
static void test5_volatile(volatile int *v, int n, volatile int *checksum) {
    /* All accesses through volatile - cannot be reordered */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
        *checksum += v[i];
    }
}

/* Test 6: Complex index calculations */
static void test6_complex_index(double *d1, double *d2, int n, volatile int *checksum) {
    for (int i = 2; i < n; i++) {
        int idx = (i * 3) % n;
        int idx2 = (i * 5) % n;
        d1[i] = d1[idx] + d2[idx2];
        *checksum += (int)d1[i];
    }
}

/* Test 7: Software pipelining candidate with multiple accumulators */
static void test7_pipeline(int *a, int *b, int *c, int n, volatile int *checksum) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Multiple accumulators to break dependencies */
    for (int i = 0; i < n; i += 3) {
        sum1 += a[i] * b[i];
        if (i + 1 < n) sum2 += a[i + 1] * b[i + 1];
        if (i + 2 < n) sum3 += a[i + 2] * b[i + 2];
        
        /* Cross-iteration dependence */
        if (i > 0) {
            c[i] = c[i - 1] + a[i];
        } else {
            c[i] = a[i];
        }
    }
    *checksum += sum1 + sum2 + sum3;
}

int main(void) {
    /* Initialize with different data patterns */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    double *d1 = (double *)malloc(SIZE * sizeof(double));
    double *d2 = (double *)malloc(SIZE * sizeof(double));
    volatile int *v = (volatile int *)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    srand(time(NULL));
    volatile int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d1[i] = (double)(rand() % 100) / 3.0;
        d2[i] = (double)(rand() % 100) / 5.0;
        v[i] = rand() % 50;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 200;
        }
    }
    
    /* Execute all tests to trigger different DDG edge creations */
    test1_flow_distance(a, b, SIZE, &checksum);
    test2_mixed_deps(a, b, c, SIZE, &checksum);
    test3_aliasing(a, b, c, SIZE);
    test3_no_restrict(a, b, SIZE);
    test4_nested(arr, &checksum);
    test5_volatile(v, SIZE, &checksum);
    test6_complex_index(d1, d2, SIZE, &checksum);
    test7_pipeline(a, b, c, SIZE, &checksum);
    
    /* Force compiler to keep all computations */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d1);
    free(d2);
    free((void *)v);
    
    return 0;
}
