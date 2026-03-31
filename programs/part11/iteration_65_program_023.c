#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define M 64
#define N 128

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, int *sum) {
    /* Flow dependence: distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
        *sum += a[i + 2];
    }
    
    /* Flow dependence: distance 1 with different data types */
    volatile float *vf = (volatile float *)a;
    for (int i = 1; i < n; i++) {
        vf[i] = vf[i - 1] + 0.5f;
        *sum += (int)vf[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n, int *sum) {
    int t;
    
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Read a[i-1] */
        a[i] = t + b[i];        /* Write a[i] - flow dependence on t, anti on a[i] */
        c[i] = c[i - 1] * 2;    /* Flow on c[i-1], output on c[i] */
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
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
            *sum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses forcing dependences */
static void test_volatile_dependence(volatile int *v, int n, int *sum) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
        *sum += v[i];
    }
}

/* Test 6: Complex addressing with mixed data types */
static void test_complex_addressing(double *d, int *idx, int n, int *sum) {
    /* Mixed float/int with indirect addressing */
    for (int i = 1; i < n; i++) {
        int j = idx[i] % n;
        d[i] = d[j] * 1.5 + d[i - 1];
        *sum += (int)d[i];
    }
}

/* Test 7: Software pipelining candidate with multiple accumulators */
__attribute__((always_inline))
static inline void test_software_pipeline(int *a, int *b, int *c, int n, int *sum) {
    int acc1 = 0, acc2 = 0;
    
    /* Loop designed to encourage modulo scheduling */
    for (int i = 0; i < n; i++) {
        acc1 += a[i] * b[i];
        c[i] = acc1 + (i > 0 ? c[i - 1] : 0);  /* Flow dependence */
        acc2 += c[i] * 2;
        
        /* Anti-dependence through accumulator reuse */
        if (i % 3 == 0) {
            a[i] = acc2;  /* WAR on a[i] */
        }
    }
    *sum += acc1 + acc2;
}

int main() {
    /* Initialize with deterministic but non-trivial pattern */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    int *idx = (int *)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int *)malloc(SIZE * sizeof(int));
    double *d = (double *)malloc(SIZE * sizeof(double));
    int (*arr)[N] = (int (*)[N])malloc(M * N * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        idx[i] = rand() % SIZE;
        v[i] = rand() % 100;
        d[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Accumulator to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, &checksum);
    test_mixed_dependences(a, b, c, SIZE, &checksum);
    test_pointer_aliasing(a, b, SIZE, &checksum);
    test_nested_loops(arr, &checksum);
    test_volatile_dependence(v, SIZE, &checksum);
    test_complex_addressing(d, idx, SIZE, &checksum);
    test_software_pipeline(a, b, c, SIZE, &checksum);
    
    /* Additional loop with varying bounds to increase coverage */
    for (int k = 0; k < 4; k++) {
        int limit = SIZE - k * 16;
        for (int i = k; i < limit; i += 2) {
            a[i] = b[i - k] + c[i + k];
            checksum += a[i];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(idx);
    free((void *)v);
    free(d);
    free(arr);
    
    return 0;
}
