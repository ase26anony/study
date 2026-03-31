#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define VOLATILE_SIZE 100

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 1, different data type */
    float *fa = (float *)a;
    float *fb = (float *)b;
    for (int i = 0; i < n - 1; i++) {
        fa[i + 1] = fa[i] + fb[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void test_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read above */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_loops(int arr[][SIZE], int m, int n) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < m; i++) {
        for (int j = 2; j < n; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Test 5: Volatile memory accesses force dependencies */
void test_volatile_dependence(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 6: Complex loop with unrolling pragma */
#pragma GCC unroll 4
void test_unrolled_loop(double *d1, double *d2, int n) {
    /* Double type, loop-carried dependence */
    for (int i = 1; i < n; i++) {
        d1[i] = d1[i - 1] * d2[i];
    }
}

/* Test 7: Anti-dependence (WAR) specific test */
void test_anti_dependence(int *a, int *b, int n) {
    for (int i = 0; i < n - 1; i++) {
        int temp = a[i];        /* Read a[i] */
        a[i] = b[i];            /* Write a[i] - anti-dependence on temp = a[i] */
        b[i] = temp;            /* Write b[i] */
    }
}

/* Test 8: Output dependence (WAW) specific test */
void test_output_dependence(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        a[i] = b[i] + 1;        /* Write a[i] */
        a[i] = a[i] * 2;        /* Write a[i] again - output dependence */
    }
}

int main() {
    /* Initialize with different patterns to avoid dead code elimination */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    double *d1 = (double *)malloc(SIZE * sizeof(double));
    double *d2 = (double *)malloc(SIZE * sizeof(double));
    volatile int *v = (volatile int *)malloc(VOLATILE_SIZE * sizeof(int));
    int (*arr)[SIZE] = (int (*)[SIZE])malloc(10 * SIZE * sizeof(int));
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d1[i] = (double)(rand() % 100) / 10.0;
        d2[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < VOLATILE_SIZE; i++) {
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < SIZE; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns */
    test_flow_dependence(a, b, SIZE);
    checksum += a[SIZE/2] + b[SIZE/2];
    
    test_mixed_dependences(a, b, c, SIZE);
    checksum += a[SIZE/3] + c[SIZE/3];
    
    test_pointer_aliasing(a, b, SIZE);
    checksum += a[SIZE/4];
    
    test_nested_loops(arr, 10, SIZE);
    checksum += arr[5][SIZE/2];
    
    test_volatile_dependence(v, VOLATILE_SIZE);
    checksum += v[VOLATILE_SIZE/2];
    
    test_unrolled_loop(d1, d2, SIZE);
    checksum += (int)d1[SIZE/2];
    
    test_anti_dependence(a, b, SIZE);
    checksum += a[SIZE/2] + b[SIZE/2];
    
    test_output_dependence(a, b, SIZE);
    checksum += a[SIZE/2];
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d1);
    free(d2);
    free((void *)v);
    free(arr);
    
    return 0;
}
