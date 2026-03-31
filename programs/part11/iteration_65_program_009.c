#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    /* Distance 2 flow dependence: a[i+2] depends on a[i] */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
        *checksum += a[i + 2];
    }
    
    /* Distance 3 anti-dependence: b[i] written after being read */
    for (int i = 3; i < n; i++) {
        int temp = b[i - 3];
        b[i] = temp * 2;
        *checksum += b[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(double *arr, float *farr, int n, int *checksum) {
    double t = 0.0;
    
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        t = arr[i - 1];              /* Flow: read arr[i-1] */
        arr[i] = t + farr[i];        /* Flow: write arr[i] depends on read */
        farr[i] = farr[i - 1] * 2.0f; /* Output: write farr[i] after read farr[i-1] */
        *checksum += (int)(arr[i] + farr[i]);
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n, int *checksum) {
    /* No restrict - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence if p overlaps */
        *checksum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_dependence(int arr[M][N], int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
static void test_volatile_dependence(volatile int *v, int n, int *checksum) {
    /* Volatile ensures memory ops can't be reordered/eliminated */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i * 2;
        *checksum += v[i];
    }
}

/* Test 6: Complex addressing with mixed data types */
static void test_complex_addressing(char *carr, short *sarr, int *iarr, int n, int *checksum) {
    for (int i = 4; i < n; i++) {
        /* Multiple interleaved dependences with different data types */
        carr[i] = carr[i - 4] + 1;              /* Distance 4, char type */
        sarr[i] = sarr[i - 2] * 2;              /* Distance 2, short type */
        iarr[i] = iarr[i - 1] + carr[i] + sarr[i]; /* Distance 1, int type */
        *checksum += carr[i] + sarr[i] + iarr[i];
    }
}

/* Test 7: Loop with if-converted dependencies */
__attribute__((always_inline))
static inline void test_predicated_dependence(int *a, int *b, int *c, int n, int *checksum) {
    for (int i = 1; i < n; i++) {
        int pred = (i % 3 == 0);
        int temp = a[i - 1];
        
        if (pred) {
            a[i] = temp + b[i];      /* Flow dependence when pred true */
            c[i] = c[i - 1] * 2;     /* Output dependence when pred true */
        } else {
            b[i] = temp - 1;         /* Anti-dependence when pred false */
        }
        
        *checksum += a[i] + b[i] + c[i];
    }
}

int main(void) {
    /* Initialize with deterministic but non-trivial values */
    srand(42);
    
    /* Allocate and initialize arrays with different data types */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    double *darr = malloc(SIZE * sizeof(double));
    float *farr = malloc(SIZE * sizeof(float));
    int *p = malloc(SIZE * sizeof(int));
    int *q = malloc(SIZE * sizeof(int));
    volatile int *v = malloc(SIZE * sizeof(int));
    char *carr = malloc(SIZE * sizeof(char));
    short *sarr = malloc(SIZE * sizeof(short));
    int *iarr = malloc(SIZE * sizeof(int));
    int arr2d[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        darr[i] = (double)(rand() % 100) / 3.0;
        farr[i] = (float)(rand() % 100) / 2.0f;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        v[i] = rand() % 100;
        carr[i] = rand() % 100;
        sarr[i] = rand() % 100;
        iarr[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, (int*)&checksum);
    test_mixed_dependences(darr, farr, SIZE, (int*)&checksum);
    test_pointer_aliasing(p, q, SIZE, (int*)&checksum);
    test_nested_dependence(arr2d, (int*)&checksum);
    test_volatile_dependence(v, SIZE, (int*)&checksum);
    test_complex_addressing(carr, sarr, iarr, SIZE, (int*)&checksum);
    test_predicated_dependence(a, b, p, SIZE, (int*)&checksum);
    
    /* Additional loop with varying bounds to increase coverage */
    for (int k = 0; k < 10; k++) {
        int limit = 50 + k * 10;
        for (int i = 1; i < limit; i++) {
            a[i] = a[i - 1] + b[i] * k;
            checksum += a[i];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(darr); free(farr);
    free(p); free(q); free((void*)v);
    free(carr); free(sarr); free(iarr);
    
    return 0;
}
