#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n) {
    /* Distance 2 flow dependence */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Distance 1 flow dependence with different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 1; i < n; i++) {
        vf[i] = vf[i - 1] + 1.5f;
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] from previous iteration */
        a[i] = t + b[i];        /* Anti: t read then written in next iteration? */
        c[i] = c[i - 1] * 2;    /* Flow: c[i-1] -> c[i] */
        b[i] = b[i] + 1;        /* Output: b[i] written multiple times */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

/* With restrict - compiler can optimize more aggressively */
static void test_restrict_pointers(int *__restrict__ rp, int *__restrict__ rq, int n) {
    for (int i = 1; i < n; i++) {
        rp[i] = rq[i] + rp[i - 1];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has distance 2 carried dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Test 5: Volatile memory forcing strong dependences */
static void test_volatile_dependence(volatile int *v, int n) {
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 6: Complex addressing with mixed data types */
static void test_complex_addressing(double *d, int *idx, int n) {
    for (int i = 1; i < n; i++) {
        d[idx[i]] = d[idx[i - 1]] * 1.5;
    }
}

/* Test 7: Loop with if-converted dependencies */
__attribute__((always_inline))
static inline void test_predicated_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        int cond = a[i] > 0;
        int t = b[i - 1];
        if (cond) {
            b[i] = t + c[i];
        } else {
            b[i] = t - c[i];
        }
        a[i] = a[i - 1] * 2;
    }
}

int main(void) {
    /* Initialize with deterministic but non-trivial data */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    int *idx = (int *)malloc(SIZE * sizeof(int));
    double *d = (double *)malloc(SIZE * sizeof(double));
    volatile int *v = (volatile int *)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        idx[i] = rand() % SIZE;
        d[i] = (double)(rand() % 100) / 10.0;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure computations aren't optimized away */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE - 10);
    checksum += a[SIZE / 2];
    
    test_mixed_dependences(a, b, c, SIZE - 20);
    checksum += b[SIZE / 3] + c[SIZE / 4];
    
    test_pointer_aliasing(a, b, SIZE - 30);
    checksum += a[SIZE / 5];
    
    test_restrict_pointers(b, c, SIZE - 40);
    checksum += b[SIZE / 6];
    
    test_nested_loops(arr);
    checksum += arr[M/2][N/2];
    
    test_volatile_dependence(v, SIZE - 50);
    checksum += v[SIZE / 7];
    
    test_complex_addressing(d, idx, SIZE - 60);
    checksum += (int)d[SIZE / 8];
    
    test_predicated_deps(a, b, c, SIZE - 70);
    checksum += a[SIZE / 9] + b[SIZE / 10];
    
    /* Additional loop to encourage modulo scheduling */
    #pragma GCC unroll 4
    for (int i = 2; i < SIZE - 2; i++) {
        /* Complex pattern with multiple interleaved dependences */
        int t1 = a[i - 2];
        int t2 = b[i - 1];
        a[i] = t1 + t2;
        b[i] = a[i] * c[i];
        c[i + 1] = b[i] - a[i - 1];
    }
    checksum += a[SIZE - 3] + b[SIZE - 2] + c[SIZE - 1];
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(idx);
    free(d);
    free((void *)v);
    
    return 0;
}
