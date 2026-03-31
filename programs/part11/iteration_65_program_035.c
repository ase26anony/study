#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 1000
#define M 100

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence_distance(int *a, int *b, int n, int *sum) {
    /* Flow dependence: a[i] written in iteration i, read in iteration i+2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    /* Anti-dependence: b[i] read, then written in same iteration */
    for (int i = 1; i < n; i++) {
        int temp = b[i - 1];
        b[i] = temp + a[i];
    }
    /* Output dependence: a[i] written twice */
    for (int i = 0; i < n - 1; i++) {
        a[i] = b[i] * 2;
        a[i] = a[i] + i;  /* WAW on a[i] */
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        *sum += a[i] + b[i];
    }
}

/* Test 2: Mixed dependence types with different data types */
__attribute__((always_inline))
static inline void test_mixed_dependences(float *fa, double *da, int n, int *sum) {
    float t = 0.0f;
    
    /* Flow + Anti + Output dependences in one loop */
    for (int i = 1; i < n; i++) {
        t = fa[i - 1];               /* Read fa[i-1] */
        fa[i] = t + (float)da[i];    /* Write fa[i] - Flow from t, Anti from fa[i] */
        da[i] = da[i - 1] * 2.0;     /* Flow on da, Output on da[i] if unrolled */
    }
    
    /* Different distance for double array */
    for (int i = 3; i < n; i++) {
        da[i] = da[i - 3] * 1.5;     /* Distance 3 flow dependence */
    }
    
    for (int i = 0; i < n; i++) {
        *sum += (int)fa[i] + (int)da[i];
    }
}

/* Test 3: Pointer aliasing with restrict and non-restrict */
static void test_pointer_aliasing(int *p, int *q, int n, int *sum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence if p overlaps */
    }
    
    /* Use restrict in inner function to create different alias scenario */
    int *restrict r1 = p;
    int *restrict r2 = q;
    for (int i = 1; i < n; i++) {
        r1[i] = r2[i - 1] + r1[i];  /* Compiler knows no aliasing */
    }
    
    for (int i = 0; i < n; i++) {
        *sum += p[i] + q[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N], int *sum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] += arr[i - 1][j];  /* Flow across outer iterations */
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            *sum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
static void test_volatile_dependences(int *sum) {
    volatile int varr[SIZE];
    volatile int vtemp;
    
    /* Simple volatile flow dependence */
    varr[0] = 1;
    for (int i = 1; i < SIZE; i++) {
        varr[i] = varr[i - 1] + i;  /* Strong memory dependence */
    }
    
    /* Volatile anti-dependence */
    for (int i = 1; i < SIZE; i++) {
        vtemp = varr[i - 1];  /* Read */
        varr[i - 1] = vtemp + i;  /* Write - WAR */
    }
    
    for (int i = 0; i < SIZE; i++) {
        *sum += varr[i];
    }
}

/* Test 6: Complex index calculations to hinder analysis */
static void test_complex_indices(int *data, int n, int *sum) {
    int indices[SIZE];
    
    /* Create non-linear access pattern */
    for (int i = 0; i < n; i++) {
        indices[i] = (i * 3 + 7) % n;
    }
    
    /* Loop with complex indexed access causing potential dependences */
    for (int i = 1; i < n; i++) {
        int idx1 = indices[i];
        int idx2 = indices[i - 1];
        data[idx1] = data[idx2] + data[i];  /* Hard to analyze dependence */
    }
    
    /* Another loop with modulo addressing */
    for (int i = 0; i < n; i++) {
        data[i % (n/2)] += data[i];  /* Potential output dependence */
    }
    
    for (int i = 0; i < n; i++) {
        *sum += data[i];
    }
}

int main(void) {
    /* Initialize with deterministic values for reproducibility */
    srand(42);
    
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    float *fa = (float*)malloc(N * sizeof(float));
    double *da = (double*)malloc(N * sizeof(double));
    int *p = (int*)malloc(N * sizeof(int));
    int *q = (int*)malloc(N * sizeof(int));
    int *data = (int*)malloc(SIZE * sizeof(int));
    int (*arr)[N] = (int(*)[N])malloc(M * N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
    }
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence_distance(a, b, N, (int*)&checksum);
    test_mixed_dependences(fa, da, N, (int*)&checksum);
    test_pointer_aliasing(p, q, N, (int*)&checksum);
    test_nested_loops(arr, (int*)&checksum);
    test_volatile_dependences((int*)&checksum);
    test_complex_indices(data, SIZE, (int*)&checksum);
    
    /* Additional loop with software pipelining characteristics */
    int temp = 0;
    #pragma GCC unroll 4
    for (int i = 2; i < N - 2; i++) {
        /* Multiple interleaved dependences */
        int x = a[i - 2];          /* Read */
        a[i] = x + b[i];           /* Write - Flow from x, Anti from a[i] */
        b[i + 1] = a[i] * 2;       /* Flow from a[i] */
        temp = b[i - 1];           /* Read */
        b[i - 1] = temp + 1;       /* Write - WAR */
    }
    
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(fa); free(da);
    free(p); free(q); free(data); free(arr);
    
    return 0;
}
