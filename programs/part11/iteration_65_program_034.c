#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test1_flow_dependence_distance(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Flow dependence with distance 3, different data type */
    volatile float *vf = (volatile float*)a;
    for (int i = 0; i < n - 3; i++) {
        vf[i + 3] = vf[i] * 1.5f;
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test2_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Read a[i-1] */
        a[i] = t + b[i];        /* Write a[i] (flow from line above, anti with next) */
        c[i] = c[i - 1] * 2;    /* Flow from c[i-1], output if c[i] written again */
        a[i - 1] = b[i] + 1;    /* Write a[i-1] (anti with first read, output if i>1) */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test3_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n] * 2;
    }
}

/* With restrict - compiler may optimize more aggressively */
static void test3_no_aliasing(int *__restrict p, int *__restrict q, int n) {
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 13) % n];  /* Self-dependence still exists */
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test4_nested_loops(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + i;
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 2; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = arr[i - 2][j] * 3;
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
static void test5_volatile_accesses(void) {
    volatile int varr[100];
    volatile int vsum = 0;
    
    /* Simple volatile flow dependence */
    for (int i = 1; i < 100; i++) {
        varr[i] = varr[i - 1] + i;
    }
    
    /* Volatile with multiple dependences */
    for (int i = 2; i < 98; i++) {
        varr[i] = varr[i - 1] + varr[i - 2];
        vsum += varr[i];
    }
}

/* Test 6: Complex addressing with mixed data types */
static void test6_complex_addressing(double *dbl_arr, int *int_arr, int n) {
    /* Mixed type accesses with potential dependences */
    for (int i = 3; i < n; i++) {
        dbl_arr[i] = dbl_arr[i - 1] * 1.5 + int_arr[i - 2];
        int_arr[i] = (int)dbl_arr[i - 3] * 2;
    }
}

/* Test 7: Loop with if-converted dependencies */
__attribute__((always_inline))
static inline void test7_predicated_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        int temp = a[i - 1];
        if (b[i] > 0) {
            a[i] = temp + c[i];      /* Flow dependence on temp */
        } else {
            a[i] = temp - c[i];      /* Also flow dependence on temp */
        }
        c[i] = b[i] * 2;             /* Output dependence on c[i] */
    }
}

/* Test 8: Reduction with carried dependence */
static int test8_reduction(int *data, int n) {
    int sum = 0;
    /* Reduction creates flow dependence through accumulator */
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[(i + 1) % n];
    }
    return sum;
}

int main(void) {
    /* Initialize with deterministic but non-trivial pattern */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *d = (int*)malloc(SIZE * sizeof(int));
    double *dbl = (double*)malloc(SIZE * sizeof(double));
    int arr2d[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        dbl[i] = (double)(rand() % 100) / 3.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test1_flow_dependence_distance(a, b, SIZE);
    checksum += a[SIZE / 2];
    
    test2_mixed_dependences(c, d, b, SIZE);
    checksum += c[SIZE / 3] + b[SIZE / 4];
    
    test3_pointer_aliasing(a, b, SIZE);
    checksum += a[SIZE / 5];
    
    test3_no_aliasing(c, d, SIZE);
    checksum += c[SIZE / 6];
    
    test4_nested_loops(arr2d);
    checksum += arr2d[M/2][N/2];
    
    test5_volatile_accesses();
    
    test6_complex_addressing(dbl, a, SIZE);
    checksum += (int)dbl[SIZE / 7];
    
    test7_predicated_deps(b, c, d, SIZE);
    checksum += b[SIZE / 8];
    
    int reduction_sum = test8_reduction(a, SIZE);
    checksum += reduction_sum;
    
    /* Additional complex loop with multiple carried dependences */
    for (int i = 4; i < SIZE - 4; i++) {
        /* Multiple interleaved dependences with different distances */
        a[i] = b[i - 1] + c[i - 2];
        b[i + 1] = a[i - 3] * 2;
        c[i + 2] = b[i - 4] + a[i - 1];
        d[i] = d[i - 2] + d[i - 4];
    }
    checksum += a[SIZE / 9] + b[SIZE / 10] + c[SIZE / 11] + d[SIZE / 12];
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(dbl);
    
    return 0;
}
