#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + i;
    }
    
    /* Flow dependence with distance 1, different data type */
    volatile float *vf = (volatile float*)a;
    for (int i = 1; i < n; i++) {
        vf[i] = vf[i - 1] + 0.5f;
    }
}

/* Test 2: Multiple dependence types in one loop */
__attribute__((always_inline))
inline void test_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Read a[i-1] */
        a[i] = t + b[i];        /* Write a[i] (flow from t, anti from a[i-1] read) */
        c[i] = c[i - 1] * 2;    /* Flow dependence on c[i-1] */
        b[i] = b[i] + 1;        /* Output dependence on b[i] */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int * __restrict p, int *q, int n) {
    /* No restrict here - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n] + i;
    }
}

void test_restrict_aliasing(int * __restrict p, int * __restrict q, int n) {
    /* Both restrict - compiler can assume no aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] * 2;
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + i + j;
        }
    }
}

/* Test 5: Volatile memory accesses forcing dependences */
void test_volatile_dependence(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + v[i] * 3;
    }
}

/* Test 6: Complex index calculations with multiple arrays */
void test_complex_indices(double *d1, double *d2, double *d3, int n) {
    /* Multiple interleaved dependences with different distances */
    for (int i = 3; i < n; i++) {
        d1[i] = d1[i - 1] * 0.5;           /* Distance 1 flow */
        d2[i] = d2[i - 2] + d1[i];         /* Distance 2 flow + immediate */
        d3[i] = d3[i - 3] * d2[i - 1];     /* Distance 3 flow + distance 1 */
    }
}

/* Test 7: Loop with if-condition creating different dependence paths */
void test_conditional_dependence(int *a, int *b, int n) {
    for (int i = 2; i < n; i++) {
        if (i % 3 == 0) {
            a[i] = a[i - 2] + b[i];    /* Distance 2 when condition true */
        } else {
            a[i] = a[i - 1] * 2;       /* Distance 1 when condition false */
        }
        b[i] = b[i - 1] + i;           /* Always distance 1 */
    }
}

int main() {
    /* Initialize with different patterns to avoid dead code elimination */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    double *d1 = (double*)malloc(SIZE * sizeof(double));
    double *d2 = (double*)malloc(SIZE * sizeof(double));
    double *d3 = (double*)malloc(SIZE * sizeof(double));
    int arr[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        v[i] = rand() % 100;
        d1[i] = (double)(rand() % 100) / 10.0;
        d2[i] = (double)(rand() % 100) / 10.0;
        d3[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger different DDG edge creations */
    test_flow_dependence(a, b, SIZE);
    test_mixed_dependences(a, b, c, SIZE);
    test_pointer_aliasing(a, b, SIZE);
    test_restrict_aliasing(b, c, SIZE);
    test_nested_loops(arr);
    test_volatile_dependence(v, SIZE);
    test_complex_indices(d1, d2, d3, SIZE);
    test_conditional_dependence(a, c, SIZE);
    
    /* Compute checksum to ensure all computations are live */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + v[i] + (int)d1[i] + (int)d2[i] + (int)d3[i];
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
    free((void*)v);
    free(d1);
    free(d2);
    free(d3);
    
    return 0;
}
