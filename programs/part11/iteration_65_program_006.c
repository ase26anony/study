#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *restrict a, int *restrict b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 1, different data type */
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
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1] */
        c[i] = c[i] + 1;        /* Output: write c[i] again */
    }
}

/* Test 3: Pointer aliasing with non-trivial indexing */
static void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict qualifier - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
        }
    }
}

/* Test 5: Volatile memory accesses forcing dependences */
static void test_volatile_dependence(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 6: Complex loop with multiple arrays and distances */
static void test_complex_loop(double *d1, double *d2, double *d3, int n) {
    /* Multiple interleaved dependences with different distances */
    for (int i = 3; i < n; i++) {
        d1[i] = d1[i - 1] + d2[i - 2];      /* Distance 1 and 2 */
        d2[i] = d3[i - 3] * 1.5;            /* Distance 3 */
        d3[i] = d1[i] + d2[i];              /* Immediate dependence */
    }
}

/* Test 7: Loop with if-condition creating varying dependences */
static void test_conditional_dependence(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        if (i % 4 == 0) {
            a[i] = a[i - 1] + b[i];     /* Flow dependence when taken */
        } else {
            b[i] = a[i] * 2;            /* Anti-dependence when not taken */
        }
    }
}

int main() {
    /* Initialize with some data */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    double *d1 = malloc(SIZE * sizeof(double));
    double *d2 = malloc(SIZE * sizeof(double));
    double *d3 = malloc(SIZE * sizeof(double));
    volatile int *v = malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d1[i] = (double)rand() / RAND_MAX;
        d2[i] = (double)rand() / RAND_MAX;
        d3[i] = (double)rand() / RAND_MAX;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE);
    test_mixed_dependences(a, b, c, SIZE);
    test_pointer_aliasing(a, b, SIZE);
    test_nested_loops(arr);
    test_volatile_dependence(v, SIZE);
    test_complex_loop(d1, d2, d3, SIZE);
    test_conditional_dependence(a, b, SIZE);
    
    /* Compute checksum to ensure all computations are live */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + (int)d1[i] + (int)d2[i] + (int)d3[i] + v[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d1);
    free(d2);
    free(d3);
    free((void*)v);
    
    return 0;
}
