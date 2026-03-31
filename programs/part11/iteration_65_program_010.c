#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, int *sum) {
    volatile int *va = (volatile int *)a;
    volatile int *vb = (volatile int *)b;
    
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        va[i + 2] = va[i] * vb[i];
    }
    
    /* Anti-dependence (WAR) */
    int temp;
    for (int i = 1; i < n; i++) {
        temp = va[i - 1];  /* Read */
        va[i] = temp + i;  /* Write - anti-dependence on va[i-1] */
    }
    
    /* Output dependence (WAW) */
    for (int i = 2; i < n; i++) {
        va[i] = va[i] * 2;     /* Write 1 */
        va[i] = va[i] + 1;     /* Write 2 - output dependence */
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += va[i];
    }
}

/* Test 2: Mixed data types for different data_type fields */
__attribute__((always_inline))
static inline void test_mixed_types(float *fa, double *da, int n, int *sum) {
    volatile float *vfa = (volatile float *)fa;
    volatile double *vda = (volatile double *)da;
    
    /* Flow dependence with floats */
    for (int i = 1; i < n; i++) {
        vfa[i] = vfa[i - 1] * 1.5f;
    }
    
    /* Flow dependence with doubles, distance 3 */
    for (int i = 0; i < n - 3; i++) {
        vda[i + 3] = vda[i] * 2.0;
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += (int)vfa[i] + (int)vda[i];
    }
}

/* Test 3: Pointer aliasing and indirect addressing */
static void test_aliasing(int *p, int *q, int n, int *sum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
    
    /* Anti-dependence with pointer arithmetic */
    int *ptr1 = p;
    int *ptr2 = q;
    for (int i = 0; i < n - 1; i++) {
        int val = *ptr1;      /* Read */
        ptr1++;
        *ptr2 = val + i;      /* Write - anti-dependence */
        ptr2++;
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += p[i] + q[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N], int *sum) {
    volatile int (*varr)[N] = (volatile int (*)[N])arr;
    
    /* Inner loop with carried dependence, distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            varr[i][j] = varr[i][j - 2] + i + j;
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < M; i++) {
        for (int j = 0; j < N; j++) {
            varr[i][j] += varr[i - 1][j];
        }
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            *sum += varr[i][j];
        }
    }
}

/* Test 5: Complex index calculations and volatile forcing */
static void test_complex_indices(int *data, int n, int *sum) {
    volatile int *vdata = (volatile int *)data;
    
    /* Multiple interleaved dependencies */
    int t1, t2;
    for (int i = 2; i < n - 2; i++) {
        t1 = vdata[i - 2];              /* Read 1 */
        t2 = vdata[i - 1];              /* Read 2 */
        vdata[i] = t1 + t2;             /* Write 1 - flow dep on both reads */
        vdata[i + 1] = vdata[i] * 3;    /* Write 2 - flow dep on write 1 */
        vdata[i - 1] = t2 + 1;          /* Write 3 - anti-dep on read 2 */
    }
    
    /* Distance 4 flow dependence */
    for (int i = 0; i < n - 4; i++) {
        vdata[i + 4] = vdata[i] + vdata[i + 1];
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += vdata[i];
    }
}

/* Test 6: Function with restrict to contrast with aliasing case */
static void test_restrict(int * restrict p, int * restrict q, int n, int *sum) {
    /* Compiler knows p and q don't alias */
    for (int i = 1; i < n; i++) {
        p[i] = p[i - 1] + q[i];  /* Only flow dependence on p */
    }
    
    /* Still has output dependence */
    for (int i = 0; i < n; i++) {
        p[i] = p[i] * 2;
        p[i] = p[i] - 1;
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += p[i];
    }
}

int main(void) {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    float *fa = (float *)malloc(SIZE * sizeof(float));
    double *da = (double *)malloc(SIZE * sizeof(double));
    int *p = (int *)malloc(SIZE * sizeof(int));
    int *q = (int *)malloc(SIZE * sizeof(int));
    int *data = (int *)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns */
    test_flow_dependence(a, b, SIZE, (int *)&checksum);
    test_mixed_types(fa, da, SIZE, (int *)&checksum);
    test_aliasing(p, q, SIZE, (int *)&checksum);
    test_nested_loops(arr, (int *)&checksum);
    test_complex_indices(data, SIZE, (int *)&checksum);
    test_restrict(p, q, SIZE, (int *)&checksum);
    
    /* Use the checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(fa);
    free(da);
    free(p);
    free(q);
    free(data);
    
    return 0;
}
