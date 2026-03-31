#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence_distance(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 3, different data type */
    volatile float *vf = (volatile float*)a;
    for (int i = 0; i < n - 3; i++) {
        vf[i + 3] = vf[i] + 1.0f;
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

/* Test 3: Pointer aliasing with restrict and non-restrict */
static void test_pointer_aliasing(int * restrict p, int *q, int n) {
    /* p is restrict, but we still have self-dependence */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

static void test_no_restrict(int *p, int *q, int n) {
    /* No restrict, compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 13) % n];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
        }
    }
}

/* Test 5: Volatile memory dependencies */
static void test_volatile_deps(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 6: Complex addressing with mixed types */
static void test_complex_addressing(double *d, int *idx, int n) {
    /* Mixed data types and indirect addressing */
    for (int i = 1; i < n; i++) {
        int j = idx[i] % n;
        d[i] = d[j] * 1.5 + d[i - 1];
    }
}

/* Test 7: Software pipelining candidate with multiple accumulators */
static void test_software_pipeline(int *a, int *b, int *c, int n) {
    int acc1 = 0, acc2 = 0;
    /* Multiple interleaved dependencies good for modulo scheduling */
    for (int i = 0; i < n; i++) {
        acc1 = acc1 + a[i];
        b[i] = acc1 + c[i];
        acc2 = acc2 * 2 - b[i];
        c[i] = acc2;
    }
}

int main(void) {
    /* Initialize with different patterns to avoid dead code elimination */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *idx = malloc(SIZE * sizeof(int));
    double *d = malloc(SIZE * sizeof(double));
    volatile int *v = malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    srand(time(NULL));
    
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
    
    volatile int checksum = 0;
    
    /* Execute all tests to create various DDG edges */
    test_flow_dependence_distance(a, b, SIZE);
    test_mixed_dependences(a, b, c, SIZE);
    test_pointer_aliasing(a, b, SIZE);
    test_no_restrict(b, c, SIZE);
    test_nested_loops(arr);
    test_volatile_deps(v, SIZE);
    test_complex_addressing(d, idx, SIZE);
    test_software_pipeline(a, b, c, SIZE);
    
    /* Compute checksum to ensure all computations are live */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + (int)d[i] + v[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(idx);
    free(d);
    free((void*)v);
    
    return 0;
}
