#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 1, different data type */
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
        t = a[i - 1];           /* Read a[i-1] */
        a[i] = t + b[i];        /* Write a[i] (flow from t, anti from a[i-1] read) */
        c[i] = c[i - 1] * 2;    /* Flow dependence on c[i-1] */
        c[i] = c[i] + 1;        /* Output dependence on c[i] */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict qualifier - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Test 5: Volatile memory accesses forcing dependencies */
static void test_volatile_dependence(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 6: Complex addressing with mixed data types */
static void test_complex_addressing(double *d, int *idx, int n) {
    for (int i = 1; i < n; i++) {
        int j = idx[i] % n;
        d[i] = d[j] * 1.5 + d[i - 1];
    }
}

/* Test 7: Loop with multiple distance values */
static void test_multiple_distances(int *a, int *b, int n) {
    /* Multiple statements with different distances */
    for (int i = 3; i < n; i++) {
        a[i] = a[i - 1] + b[i];     /* distance 1 */
        b[i] = b[i - 2] * 2;        /* distance 2 */
        a[i] = a[i - 3] + a[i];     /* distance 3 */
    }
}

int main() {
    /* Initialize with different patterns to avoid dead code elimination */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *idx = malloc(SIZE * sizeof(int));
    double *d = malloc(SIZE * sizeof(double));
    volatile int *v = malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
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
    
    /* Execute all test patterns to trigger different DDG edge creations */
    test_flow_dependence(a, b, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += a[i];
    
    test_mixed_dependences(a, b, c, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += b[i] + c[i];
    
    test_pointer_aliasing(a, b, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += a[i];
    
    test_nested_loops(arr);
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr[i][j];
        }
    }
    
    test_volatile_dependence(v, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += v[i];
    
    test_complex_addressing(d, idx, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += (int)d[i];
    
    test_multiple_distances(a, b, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += a[i] + b[i];
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(idx);
    free(d);
    free((void*)v);
    
    return 0;
}
