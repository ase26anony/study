#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Simple loop-carried flow dependence with distance 2 */
__attribute__((always_inline))
inline void test1_flow_dependence_distance2(int *a, int *b, int n, volatile int *checksum) {
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];  /* Flow dependence, distance 2 */
        *checksum += a[i + 2];
    }
}

/* Test 2: Multiple dependence types in one loop */
__attribute__((always_inline))
inline void test2_mixed_dependences(int *a, int *b, int *c, int n, volatile int *checksum) {
    int t;
    for (int i = 1; i < n; i++) {
        t = a[i - 1];            /* Read a[i-1] */
        a[i] = t + b[i];         /* Write a[i] - Flow dependence with previous read */
        c[i] = c[i - 1] * 2;     /* Output dependence on c[i-1] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Anti-dependence (WAR) with volatile */
__attribute__((always_inline))
inline void test3_anti_dependence_volatile(volatile int *v, int *b, int n, volatile int *checksum) {
    int temp;
    for (int i = 1; i < n; i++) {
        temp = v[i - 1];         /* Read v[i-1] */
        v[i] = temp + b[i];      /* Write v[i] - Anti-dependence on v[i-1] read */
        *checksum += v[i];
    }
}

/* Test 4: Indirect addressing with potential aliasing */
void test4_indirect_addressing(int *p, int *q, int n, volatile int *checksum) {
    /* No restrict keyword - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence due to p self-overlap */
        *checksum += p[i];
    }
}

/* Test 5: Nested loop with inner loop carried dependence */
__attribute__((always_inline))
inline void test5_nested_loop_carried(int arr[M][N], volatile int *checksum) {
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;  /* Flow dependence, distance 2 in inner loop */
            *checksum += arr[i][j];
        }
    }
}

/* Test 6: Different data types to affect data_type field */
__attribute__((always_inline))
inline void test6_mixed_data_types(float *fa, double *da, int *ia, int n, volatile int *checksum) {
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;    /* Float flow dependence */
        da[i] = da[i - 1] * 2.0;     /* Double flow dependence */
        ia[i] = ia[i - 1] + i;       /* Integer flow dependence */
        *checksum += (int)(fa[i] + da[i] + ia[i]);
    }
}

/* Test 7: Complex loop with multiple distances */
__attribute__((always_inline))
inline void test7_multiple_distances(int *a, int *b, int *c, int n, volatile int *checksum) {
    for (int i = 3; i < n; i++) {
        a[i] = a[i - 1] + b[i];      /* Distance 1 flow */
        c[i] = c[i - 3] * a[i - 2];  /* Distance 3 flow, also depends on a[i-2] */
        *checksum += a[i] + c[i];
    }
}

/* Test 8: Output dependence (WAW) pattern */
__attribute__((always_inline))
inline void test8_output_dependence(int *a, int *b, int n, volatile int *checksum) {
    for (int i = 1; i < n; i++) {
        a[i] = b[i] * 2;      /* Write a[i] */
        a[i] = a[i] + 1;      /* Write a[i] again - output dependence */
        a[i - 1] = b[i] * 3;  /* Write a[i-1] - possible output with next iteration */
        *checksum += a[i];
    }
}

/* Test 9: Restrict vs non-restrict comparison */
void test9_restrict_aliasing(int * __restrict r1, int * __restrict r2, 
                            int *p, int *q, int n, volatile int *checksum) {
    /* Restrict pointers - compiler can assume no aliasing */
    for (int i = 1; i < n; i++) {
        r1[i] = r1[i - 1] + r2[i];
        *checksum += r1[i];
    }
    
    /* Non-restrict pointers - compiler must be conservative */
    for (int i = 1; i < n; i++) {
        p[i] = p[i - 1] + q[i];
        *checksum += p[i];
    }
}

int main() {
    /* Initialize with deterministic values for reproducibility */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    int *ia = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        ia[i] = rand() % 100;
        if (i < SIZE) v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test1_flow_dependence_distance2(a, b, SIZE, &checksum);
    test2_mixed_dependences(a, b, c, SIZE, &checksum);
    test3_anti_dependence_volatile(v, b, SIZE, &checksum);
    test4_indirect_addressing(a, b, SIZE, &checksum);
    test5_nested_loop_carried(arr, &checksum);
    test6_mixed_data_types(fa, da, ia, SIZE, &checksum);
    test7_multiple_distances(a, b, c, SIZE, &checksum);
    test8_output_dependence(a, b, SIZE, &checksum);
    test9_restrict_aliasing(a, b, c, ia, SIZE, &checksum);
    
    /* Print checksum to ensure all computations are live */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    free(ia);
    free((void*)v);
    
    return 0;
}
