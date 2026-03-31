#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test1_flow_dependence_distance(int *a, int *b, int n, int *checksum) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
        *checksum += a[i + 2];
    }
    
    /* Flow dependence with distance 1, different data type */
    volatile float *vf = (volatile float*)a;
    for (int i = 0; i < n - 1; i++) {
        vf[i + 1] = vf[i] * 1.5f;
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void test2_mixed_dependences(int *a, int *b, int *c, int n, int *checksum) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Flow: read a[i-1] */
        a[i] = t + b[i];            /* Flow: use t, Anti: write a[i] after read */
        c[i] = c[i - 1] * 2;        /* Flow: read c[i-1], Output: write c[i] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test3_pointer_aliasing(int *p, int *q, int n, int *checksum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
        *checksum += p[i];
    }
}

/* With restrict - compiler can assume no aliasing */
void test3_noalias(int *restrict p, int *restrict q, int n, int *checksum) {
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + i;
        *checksum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test4_nested_loops(int arr[M][N], int *checksum) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses forcing dependences */
void test5_volatile_dependence(volatile int *v, int n, int *checksum) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
        *checksum += v[i];
    }
}

/* Test 6: Complex loop with multiple arrays and indices */
void test6_complex_pattern(double *d1, double *d2, double *d3, int n, int *checksum) {
    /* Mix of different distance dependences */
    for (int i = 3; i < n; i++) {
        d1[i] = d2[i - 1] + d3[i - 3];      /* Distance 1 and 3 */
        d2[i] = d1[i - 2] * 0.5;            /* Distance 2 */
        *checksum += (int)(d1[i] + d2[i]);
    }
}

/* Test 7: Loop with if-condition creating control dependence */
void test7_control_dependent(int *a, int *b, int *c, int n, int *checksum) {
    for (int i = 1; i < n; i++) {
        if (b[i] > 0) {
            a[i] = a[i - 1] + c[i];    /* Flow dependence when taken */
        } else {
            a[i] = b[i];               /* Different path */
        }
        *checksum += a[i];
    }
}

/* Main driver that calls all tests */
int main() {
    /* Initialize with deterministic values for reproducibility */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    double *d1 = (double*)malloc(SIZE * sizeof(double));
    double *d2 = (double*)malloc(SIZE * sizeof(double));
    double *d3 = (double*)malloc(SIZE * sizeof(double));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d1[i] = (double)(rand() % 100) / 10.0;
        d2[i] = (double)(rand() % 100) / 10.0;
        d3[i] = (double)(rand() % 100) / 10.0;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Accumulator to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger different DDG edge creations */
    test1_flow_dependence_distance(a, b, SIZE, &checksum);
    test2_mixed_dependences(a, b, c, SIZE, &checksum);
    test3_pointer_aliasing(a, b, SIZE, &checksum);
    test3_noalias(c, b, SIZE, &checksum);
    test4_nested_loops(arr, &checksum);
    test5_volatile_dependence(v, SIZE, &checksum);
    test6_complex_pattern(d1, d2, d3, SIZE, &checksum);
    test7_control_dependent(a, b, c, SIZE, &checksum);
    
    /* Additional loop with software pipelining characteristics */
    for (int i = 4; i < SIZE - 4; i++) {
        /* Multiple interleaved dependences */
        a[i] = b[i - 2] + c[i - 1];
        b[i] = a[i - 3] * 2;
        c[i] = b[i - 1] + a[i - 4];
        checksum += a[i] + b[i] + c[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
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
