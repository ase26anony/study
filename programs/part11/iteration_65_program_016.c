#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence: distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence: distance 1 with different data type */
    volatile float *vf = (volatile float*)a;
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
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

/* Test 4: With restrict - allows more optimization */
static void test_restrict_pointers(int *__restrict__ rp, int *__restrict__ rq, int n) {
    for (int i = 0; i < n; i++) {
        rp[i] = rq[i] + i;
    }
}

/* Test 5: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has distance 2 dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Test 6: Volatile memory accesses to force dependences */
static void test_volatile_access(volatile int *v, int n) {
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 7: Complex index calculations */
static void test_complex_indices(double *d, int n) {
    for (int i = 0; i < n; i++) {
        int idx = (i * 3 + 7) % n;
        d[i] = d[idx] * 0.5;
    }
}

/* Test 8: Loop with if condition creating varying dependences */
static void test_conditional_dep(int *a, int *b, int n) {
    for (int i = 2; i < n; i++) {
        if (i % 3 == 0) {
            a[i] = a[i - 2] + b[i];
        } else {
            a[i] = a[i - 1] * 2;
        }
    }
}

int main() {
    /* Initialize with different patterns */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    volatile int *v = malloc(SIZE * sizeof(int));
    double *d = malloc(SIZE * sizeof(double));
    int arr[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        v[i] = rand() % 100;
        d[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to create various DDG edges */
    test_flow_dependence(a, b, SIZE);
    checksum += a[SIZE/2];
    
    test_mixed_dependences(a, b, c, SIZE);
    checksum += b[SIZE/3] + c[SIZE/4];
    
    test_pointer_aliasing(a, b, SIZE);
    checksum += a[SIZE/5];
    
    test_restrict_pointers(a, b, SIZE);
    checksum += a[SIZE/6];
    
    test_nested_loops(arr);
    checksum += arr[M/2][N/2];
    
    test_volatile_access(v, SIZE);
    checksum += v[SIZE/7];
    
    test_complex_indices(d, SIZE);
    checksum += (int)d[SIZE/8];
    
    test_conditional_dep(a, b, SIZE);
    checksum += a[SIZE/9];
    
    /* Final computation using all results */
    int final_result = 0;
    for (int i = 0; i < SIZE; i += 16) {
        final_result += a[i] + b[i] + c[i] + (int)d[i];
    }
    
    printf("Checksum: %d, Final: %d\n", checksum, final_result);
    
    free(a);
    free(b);
    free(c);
    free((void*)v);
    free(d);
    
    return 0;
}
