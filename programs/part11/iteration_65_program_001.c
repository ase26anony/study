#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    /* Distance 2 flow dependence */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
        *checksum += a[i + 2];
    }
    
    /* Distance 1 flow dependence with different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 1; i < n; i++) {
        vf[i] = vf[i - 1] + 0.5f;
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void test_mixed_dependences(int *a, int *b, int *c, int n, int *checksum) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Flow: read a[i-1] */
        a[i] = t + b[i];            /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;        /* Flow: read c[i-1], Output: write c[i] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *p, int *q, int n, int *checksum) {
    /* No restrict - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
        *checksum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_dependence(int arr[M][N], int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses forcing dependencies */
void test_volatile_dependence(volatile int *v, int n, int *checksum) {
    /* Strong memory dependencies that cannot be optimized away */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
        *checksum += v[i];
    }
}

/* Test 6: Complex index calculations with multiple arrays */
void test_complex_indices(double *d1, double *d2, double *d3, int n, int *checksum) {
    /* Multiple interleaved dependencies with floating point */
    for (int i = 2; i < n - 2; i++) {
        d1[i] = d2[i - 1] + d3[i + 1];      /* Flow from d2 and d3 */
        d2[i] = d1[i - 2] * 1.5;            /* Flow from d1 with distance 2 */
        d3[i] = d3[i - 1] + d2[i];          /* Flow from d3 and d2 */
        *checksum += (int)(d1[i] + d2[i] + d3[i]);
    }
}

/* Test 7: Software pipelining candidate with long latency chain */
void test_long_latency_chain(int *a, int *b, int *c, int n, int *checksum) {
    /* Multiple dependent operations creating long chains */
    for (int i = 3; i < n; i++) {
        int t1 = a[i - 1] * b[i - 2];      /* Distance 1 from a, 2 from b */
        int t2 = t1 + c[i - 3];            /* Distance 3 from c */
        a[i] = t2 * 7;                     /* Output to a[i] */
        b[i] = a[i - 1] + t2;              /* Anti-dependence on a[i-1] */
        *checksum += a[i] + b[i];
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
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, (int*)&checksum);
    test_mixed_dependences(a, b, c, SIZE, (int*)&checksum);
    test_pointer_aliasing(a, b, SIZE / 2, (int*)&checksum);
    test_nested_dependence(arr, (int*)&checksum);
    test_volatile_dependence(v, SIZE, (int*)&checksum);
    test_complex_indices(d1, d2, d3, SIZE, (int*)&checksum);
    test_long_latency_chain(a, b, c, SIZE, (int*)&checksum);
    
    /* Use checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(d1); free(d2); free(d3);
    free((void*)v);
    
    return 0;
}
