#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 1000
#define M 100

/* Force inlining to create larger basic blocks */
__attribute__((always_inline)) 
static inline int compute(int a, int b) {
    return a * 3 + b * 7;
}

/* Test 1: Loop-carried flow dependence with distance > 0 */
void test1_flow_dependence_distance(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Flow dependence with distance 1, mixed with anti-dependence */
    volatile int temp; /* volatile to prevent optimization */
    for (int i = 1; i < n; i++) {
        temp = a[i - 1];  /* Read a[i-1] */
        a[i] = temp + b[i]; /* Write a[i] - creates anti-dependence with next iteration */
    }
}

/* Test 2: Multiple dependence types in single loop */
void test2_mixed_dependences(int *a, int *b, int *c, int n) {
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        int t = a[i - 1];           /* Flow: read a[i-1] from previous iteration */
        a[i] = t + b[i];            /* Write a[i] - creates output dep with next iteration */
        c[i] = c[i - 1] * 2;        /* Flow: read c[i-1], write c[i] */
        b[i - 1] = c[i] + 1;        /* Anti: read c[i] just written, write b[i-1] */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test3_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict keyword - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        /* Complex indexing creates potential flow dependence */
        p[i] = q[i] + p[(i * 7) % n] + p[(i * 3 + 1) % n];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
void test4_nested_loops(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + 1;
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = arr[i - 1][j] * 2;
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
void test5_volatile_dependences(volatile int *v, int n) {
    /* Volatile guarantees memory accesses aren't reordered */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
    
    /* Multiple volatile accesses with different distances */
    for (int i = 2; i < n - 2; i++) {
        v[i] = v[i - 2] * v[i - 1] + v[i + 1];
    }
}

/* Test 6: Different data types for different edge data types */
void test6_mixed_data_types(float *fa, double *da, int *ia, int n) {
    /* Float array with flow dependence */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;
    }
    
    /* Double array with anti-dependence */
    for (int i = 0; i < n - 1; i++) {
        double temp = da[i];
        da[i] = ia[i] * 2.0;
        ia[i] = (int)temp;
    }
}

/* Test 7: Complex loop with unrolling hint */
#pragma GCC unroll 4
void test7_unrolled_loop(int *a, int *b, int *c, int n) {
    /* Loop designed to be unrolled, creating more complex DDG */
    for (int i = 4; i < n; i++) {
        a[i] = b[i - 1] + c[i - 2];
        b[i] = a[i - 3] * 2;
        c[i] = b[i - 4] + a[i - 1];
    }
}

/* Test 8: Reduction with carried dependence */
void test8_reduction(int *a, int n) {
    int sum = 0;
    /* Reduction creates flow dependence through accumulator */
    for (int i = 0; i < n; i++) {
        sum += a[i];
        a[i] = sum;
    }
}

int main() {
    /* Initialize with different patterns to avoid dead code elimination */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
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
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test1_flow_dependence_distance(a, b, SIZE);
    for (int i = 0; i < 10; i++) checksum += a[i];
    
    test2_mixed_dependences(a, b, c, SIZE);
    for (int i = 0; i < 10; i++) checksum += b[i] + c[i];
    
    test3_pointer_aliasing(a, b, SIZE);
    for (int i = 0; i < 10; i++) checksum += a[i];
    
    test4_nested_loops(arr);
    for (int i = 0; i < 5; i++) checksum += arr[i][0];
    
    test5_volatile_dependences(v, SIZE);
    for (int i = 0; i < 10; i++) checksum += v[i];
    
    test6_mixed_data_types(fa, da, a, SIZE);
    for (int i = 0; i < 10; i++) checksum += (int)fa[i] + (int)da[i];
    
    test7_unrolled_loop(a, b, c, SIZE);
    for (int i = 0; i < 10; i++) checksum += a[i] + b[i] + c[i];
    
    test8_reduction(a, SIZE);
    for (int i = 0; i < 10; i++) checksum += a[i];
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    free((void*)v);
    
    return 0;
}
