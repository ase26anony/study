#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Flow dependence with distance 2 */
__attribute__((always_inline))
inline void test_flow_distance(int *a, int *b, int n) {
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];  /* Flow dependence, distance 2 */
    }
}

/* Test 2: Multiple dependence types in one loop */
__attribute__((always_inline))
inline void test_mixed_deps(int *a, int *b, int *c, int n) {
    int t;
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow dependence (RAW) from previous iteration */
        a[i] = t + b[i];        /* Anti-dependence (WAR) on 't' */
        c[i] = c[i - 1] * 2;    /* Flow dependence on c[i-1] */
        b[i] = b[i] * 3;        /* Output dependence (WAW) on b[i] */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict qualifiers - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
}

/* Test 4: Nested loop with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_loop(int arr[M][N]) {
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;  /* Inner loop carried dependence, distance 2 */
        }
    }
}

/* Test 5: Volatile memory accesses to force dependencies */
void test_volatile_deps(volatile int *v, int n) {
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;  /* Volatile ensures memory dependence */
    }
}

/* Test 6: Different data types for varied data_type field */
__attribute__((always_inline))
inline void test_mixed_types(float *fa, double *da, int *ia, int n) {
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;      /* float flow dependence */
        da[i] = da[i - 1] * 2.0;       /* double flow dependence */
        ia[i] = ia[i - 1] + i;         /* int flow dependence */
    }
}

/* Test 7: Complex index calculations with modulo */
void test_complex_indices(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 3 + 7) % n;
        int idx2 = (i * 5 + 11) % n;
        arr[idx1] = arr[idx2] + arr[i];  /* Complex addressing may create various edges */
    }
}

/* Test 8: Loop with multiple distances */
void test_multiple_distances(int *a, int *b, int n) {
    for (int i = 4; i < n; i++) {
        a[i] = a[i - 1] + 1;      /* distance 1 */
        b[i] = b[i - 2] * 2;      /* distance 2 */
        a[i] += a[i - 3];         /* distance 3 */
        b[i] += b[i - 4];         /* distance 4 */
    }
}

int main() {
    /* Initialize with different patterns */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    float *fa = malloc(SIZE * sizeof(float));
    double *da = malloc(SIZE * sizeof(double));
    volatile int *v = malloc(SIZE * sizeof(int));
    int arr2d[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all test patterns to create various DDG edges */
    test_flow_distance(a, b, SIZE);
    checksum += a[SIZE-1];
    
    test_mixed_deps(a, b, c, SIZE);
    checksum += b[SIZE-1] + c[SIZE-1];
    
    test_pointer_aliasing(a, b, SIZE);
    checksum += a[SIZE/2];
    
    test_nested_loop(arr2d);
    checksum += arr2d[M-1][N-1];
    
    test_volatile_deps(v, SIZE);
    checksum += v[SIZE-1];
    
    test_mixed_types(fa, da, a, SIZE);
    checksum += (int)fa[SIZE-1] + (int)da[SIZE-1];
    
    test_complex_indices(b, SIZE);
    checksum += b[SIZE/3];
    
    test_multiple_distances(a, b, SIZE);
    checksum += a[SIZE-1] + b[SIZE-1];
    
    /* Final computation to ensure all results are used */
    int final_result = checksum % 1000;
    printf("Result: %d\n", final_result);
    
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    free((void*)v);
    
    return final_result;
}
