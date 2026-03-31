#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence: distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence: distance 1 with different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 0; i < n - 1; i++) {
        vf[i + 1] = vf[i] + 1.5f;
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: write a[i], Anti: read t */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], write c[i] */
        b[i] = b[i] + 1;        /* Output: write b[i] */
    }
}

/* Test 3: Indirect addressing with potential aliasing */
static void test_indirect_addressing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

/* Test 4: With restrict - compiler can assume no aliasing */
static void test_restrict_pointers(int *__restrict__ rp, int *__restrict__ rq, int n) {
    for (int i = 0; i < n; i++) {
        rp[i] = rq[i] + i;
    }
}

/* Test 5: Nested loop with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loop(int arr[M][N]) {
    /* Inner loop has distance 2 dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Test 6: Volatile memory accesses to force dependences */
static void test_volatile_access(volatile int *v, int n) {
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 7: Complex pointer arithmetic and loop unrolling */
static void test_pointer_arithmetic(int *data, int n) {
    int *p = data;
    int *end = data + n;
    
    #pragma GCC unroll 2
    while (p < end - 3) {
        p[3] = p[0] + p[1] + p[2];
        p += 1;
    }
}

/* Test 8: Mixed data types for different edge data_type fields */
static void test_mixed_data_types(double *darr, float *farr, int *iarr, int n) {
    for (int i = 1; i < n; i++) {
        darr[i] = darr[i - 1] * 1.01;      /* double flow dependence */
        farr[i] = farr[i - 1] + 0.5f;      /* float flow dependence */
        iarr[i] = iarr[i - 1] + i;         /* int flow dependence */
    }
}

/* Test 9: Loop with if condition creating control dependences */
static void test_conditional_dependence(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        if (b[i] > 0) {
            a[i] = a[i - 1] + b[i];    /* Flow dependence when taken */
        } else {
            a[i] = a[i] * 2;           /* Output dependence */
        }
    }
}

int main(void) {
    /* Initialize with different sizes to create varied DDGs */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    double *darr = (double *)malloc(SIZE * sizeof(double));
    float *farr = (float *)malloc(SIZE * sizeof(float));
    volatile int *varr = (volatile int *)malloc(SIZE * sizeof(int));
    int arr2d[M][N];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        darr[i] = (double)(rand() % 100) / 10.0;
        farr[i] = (float)(rand() % 100) / 10.0f;
        varr[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += a[i];
    
    test_mixed_dependences(a, b, c, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += b[i] + c[i];
    
    test_indirect_addressing(a, b, SIZE);
    test_restrict_pointers(c, a, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += c[i];
    
    test_nested_loop(arr2d);
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    test_volatile_access(varr, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += varr[i];
    
    test_pointer_arithmetic(a, SIZE);
    test_mixed_data_types(darr, farr, b, SIZE);
    test_conditional_dependence(c, a, SIZE);
    
    /* Final checksum computation */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + (int)darr[i] + (int)farr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(darr);
    free(farr);
    free((int *)varr);
    
    return 0;
}
