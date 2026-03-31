#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, int *sum) {
    /* Flow dependence: distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
        *sum += a[i + 2];
    }
    
    /* Flow dependence: distance 1 with different data types */
    float *fa = (float*)a;
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] + 1.5f;
        *sum += (int)fa[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n, int *sum) {
    int t = 0;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: a[i] written after potential read */
        c[i] = c[i - 1] * 2;    /* Flow: c[i-1] -> c[i] */
        c[i] += i;              /* Output: c[i] written again */
        *sum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n, int *sum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
        *sum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N], int *sum) {
    /* Inner loop has distance 2 dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;  /* Flow dependence distance 2 */
            *sum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
static void test_volatile_dependence(volatile int *v, int n, int *sum) {
    /* Volatile ensures memory ops aren't reordered/eliminated */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;  /* Strong flow dependence */
        *sum += v[i];
    }
}

/* Test 6: Complex addressing with mixed data types */
static void test_complex_addressing(double *d, int *idx, int n, int *sum) {
    /* Mixed float/int with indirect addressing */
    for (int i = 2; i < n; i++) {
        int j = idx[i] % n;
        d[i] = d[j] * 1.5 + d[i - 2];  /* Two flow dependences */
        *sum += (int)d[i];
    }
}

/* Test 7: Software pipelining candidate with multiple accumulators */
__attribute__((always_inline))
static inline void test_software_pipeline(int *a, int *b, int *c, int n, int *sum) {
    /* Multiple interleaved dependences - good for modulo scheduling */
    int acc1 = 0, acc2 = 0;
    for (int i = 1; i < n; i++) {
        acc1 = a[i - 1] + b[i];      /* Flow: a[i-1] */
        a[i] = acc1 * 2;             /* Anti: a[i] written */
        acc2 = c[i] + acc1;          /* Flow: acc1 */
        c[i - 1] = acc2;             /* Flow: c[i-1] written for next iter? */
        *sum += a[i] + c[i - 1];
    }
}

/* Test 8: Reduction with loop-carried dependence */
static void test_reduction(double *d, int n, int *sum) {
    double total = 0.0;
    for (int i = 0; i < n; i++) {
        total += d[i];  /* Flow dependence through reduction variable */
        d[i] = total;   /* Output dependence + flow from total */
        *sum += (int)total;
    }
}

int main(void) {
    /* Initialize with deterministic but non-trivial data */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *idx = malloc(SIZE * sizeof(int));
    double *d = malloc(SIZE * sizeof(double));
    volatile int *v = malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        idx[i] = rand() % SIZE;
        d[i] = (double)(rand() % 100) / 3.0;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, &checksum);
    test_mixed_dependences(a, b, c, SIZE, &checksum);
    test_pointer_aliasing(a, b, SIZE, &checksum);
    test_nested_loops(arr, &checksum);
    test_volatile_dependence(v, SIZE, &checksum);
    test_complex_addressing(d, idx, SIZE, &checksum);
    test_software_pipeline(a, b, c, SIZE, &checksum);
    test_reduction(d, SIZE, &checksum);
    
    /* Additional iterations to increase optimization opportunities */
    for (int iter = 0; iter < 3; iter++) {
        test_flow_dependence(a, b, SIZE, &checksum);
        test_mixed_dependences(a, b, c, SIZE, &checksum);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(idx); free(d); free((void*)v);
    
    return 0;
}
