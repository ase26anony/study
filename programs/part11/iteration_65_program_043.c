#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance 2 */
__attribute__((always_inline))
inline void test1_flow_distance2(int *a, int *b, int n, volatile int *checksum) {
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];  /* Flow dependence, distance 2 */
        *checksum += a[i + 2];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void test2_mixed_deps(int *a, int *b, int *c, int n, volatile int *checksum) {
    int t;
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow dependence (RAW) from previous iteration */
        a[i] = t + b[i];        /* Anti-dependence (WAR) on 't' */
        c[i] = c[i - 1] * 2;    /* Flow dependence on c[i-1] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Output dependence (WAW) with volatile */
__attribute__((always_inline))
inline void test3_output_dep(volatile int *v, int n, volatile int *checksum) {
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;    /* Flow dependence + potential WAW */
        v[i] = v[i] * 2;        /* WAW on v[i] */
        *checksum += v[i];
    }
}

/* Test 4: Indirect addressing with possible aliasing */
void test4_indirect_addressing(int *p, int *q, int n, volatile int *checksum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
        *checksum += p[i];
    }
}

/* Test 5: Nested loop with inner loop carried dependence */
__attribute__((always_inline))
inline void test5_nested_loop(int arr[M][N], volatile int *checksum) {
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;  /* Flow dependence, distance 2 */
            *checksum += arr[i][j];
        }
    }
}

/* Test 6: Different data types to affect data_type field */
__attribute__((always_inline))
inline void test6_mixed_types(float *fa, double *da, int n, volatile int *checksum) {
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;      /* Float flow dependence */
        da[i] = da[i - 1] * 2.0;       /* Double flow dependence */
        *checksum += (int)(fa[i] + da[i]);
    }
}

/* Test 7: Complex pointer arithmetic with loop-carried dependence */
void test7_complex_pointer(int *base, int n, volatile int *checksum) {
    int *p = base;
    int *q = base + n/2;
    
    for (int i = 0; i < n/2; i++) {
        p[i] = q[i] + p[(i + 1) % (n/2)];  /* Flow to next iteration */
        *checksum += p[i];
    }
}

/* Test 8: Anti-dependence (WAR) with register pressure */
__attribute__((always_inline))
inline void test8_anti_dependence(int *a, int *b, int n, volatile int *checksum) {
    int temp;
    for (int i = 0; i < n - 1; i++) {
        temp = a[i];            /* Read a[i] */
        a[i] = b[i] + 1;        /* Write a[i] - WAR on a[i] */
        b[i] = temp;            /* Write b[i] */
        *checksum += a[i] + b[i];
    }
}

int main() {
    /* Initialize with some data */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
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
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test1_flow_distance2(a, b, SIZE, &checksum);
    test2_mixed_deps(a, b, c, SIZE, &checksum);
    test3_output_dep(v, SIZE, &checksum);
    test4_indirect_addressing(a, b, SIZE, &checksum);
    test5_nested_loop(arr, &checksum);
    test6_mixed_types(fa, da, SIZE, &checksum);
    test7_complex_pointer(a, SIZE, &checksum);
    test8_anti_dependence(a, b, SIZE, &checksum);
    
    /* Use results to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    free((void*)v);
    
    return 0;
}
