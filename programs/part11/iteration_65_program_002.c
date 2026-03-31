#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 1000
#define M 100

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, int *sum) {
    /* Flow dependence: distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Anti-dependence: distance 1 */
    int temp;
    for (int i = 1; i < n; i++) {
        temp = a[i - 1];  /* Read a[i-1] */
        a[i] = temp + b[i]; /* Write a[i] - WAR */
    }
    
    /* Output dependence: distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i - 1] * 2; /* WAW */
    }
    
    /* Accumulate results */
    for (int i = 0; i < n; i++) {
        *sum += a[i];
    }
}

/* Test 2: Mixed dependence types with different data types */
__attribute__((always_inline))
static inline void test_mixed_dependences(float *fa, double *da, int *ia, int n, int *sum) {
    /* Flow dependence with float */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;
    }
    
    /* Anti-dependence with double */
    double tmp;
    for (int i = 1; i < n; i++) {
        tmp = da[i - 1];    /* Read */
        da[i] = tmp + 0.5;  /* Write - WAR */
    }
    
    /* Output dependence with int */
    for (int i = 1; i < n; i++) {
        ia[i] = ia[i - 1] + 1; /* WAW */
    }
    
    /* Complex mixed pattern */
    for (int i = 2; i < n; i++) {
        float f1 = fa[i - 2];
        double d1 = da[i - 1];
        ia[i] = (int)(f1 * d1) + ia[i - 1];
        fa[i] = f1 + (float)d1;
        da[i] = d1 * 0.9;
    }
    
    for (int i = 0; i < n; i++) {
        *sum += (int)fa[i] + (int)da[i] + ia[i];
    }
}

/* Test 3: Pointer aliasing and indirect addressing */
static void test_pointer_aliasing(int *p, int *q, int n, int *sum) {
    /* Potential aliasing - no restrict keyword */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n]; /* Possible flow dependence */
    }
    
    /* Pointer arithmetic */
    int *ptr1 = p;
    int *ptr2 = q;
    for (int i = 0; i < n - 1; i++) {
        *(ptr1 + i + 1) = *(ptr2 + i) + *(ptr1 + i); /* Flow dependence */
    }
    
    for (int i = 0; i < n; i++) {
        *sum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N], int *sum) {
    /* Inner loop with distance 2 dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1; /* Flow dependence, distance 2 */
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = arr[i - 1][j] * 2; /* Flow dependence across outer loop */
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            *sum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependencies */
static void test_volatile_access(int *sum) {
    volatile int varr[SIZE];
    
    /* Simple volatile flow dependence */
    for (int i = 1; i < SIZE; i++) {
        varr[i] = varr[i - 1] + i;
    }
    
    /* Volatile with multiple dependencies */
    volatile int v1[SIZE], v2[SIZE];
    for (int i = 2; i < SIZE; i++) {
        v2[i] = v1[i - 1];      /* Anti-dependence on v1 */
        v1[i] = v2[i - 2] + 1;  /* Flow dependence on v2, distance 2 */
    }
    
    for (int i = 0; i < SIZE; i++) {
        *sum += varr[i] + v1[i] + v2[i];
    }
}

/* Test 6: Complex loop with multiple interleaved dependencies */
static void test_complex_pattern(int *a, int *b, int *c, int n, int *sum) {
    /* Multiple interleaved dependencies */
    for (int i = 2; i < n; i++) {
        int t1 = a[i - 2];          /* Read a[i-2] */
        b[i] = t1 + c[i - 1];       /* Flow from a, Flow from c */
        a[i] = b[i - 1] * 2;        /* Flow from b, Output on a */
        c[i] = a[i - 1] + b[i - 2]; /* Flow from a and b */
    }
    
    /* Another pattern with different distance */
    for (int i = 3; i < n; i++) {
        a[i] = a[i - 3] + b[i - 2] - c[i - 1];
        b[i] = a[i - 1] * b[i - 3];
        c[i] = c[i - 2] + a[i - 1];
    }
    
    for (int i = 0; i < n; i++) {
        *sum += a[i] + b[i] + c[i];
    }
}

int main() {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    float *fa = (float*)malloc(N * sizeof(float));
    double *da = (double*)malloc(N * sizeof(double));
    int *ia = (int*)malloc(N * sizeof(int));
    int arr[M][N];
    
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        ia[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure computations aren't optimized away */
    volatile int checksum = 0;
    
    /* Execute all test patterns */
    test_flow_dependence(a, b, N, (int*)&checksum);
    test_mixed_dependences(fa, da, ia, N, (int*)&checksum);
    test_pointer_aliasing(a, b, N, (int*)&checksum);
    test_nested_loops(arr, (int*)&checksum);
    test_volatile_access((int*)&checksum);
    test_complex_pattern(a, b, c, N, (int*)&checksum);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    free(ia);
    
    return 0;
}
