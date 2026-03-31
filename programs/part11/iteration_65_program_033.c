#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    /* Distance 2 flow dependence */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Accumulate results */
    for (int i = 0; i < n; i++) {
        *checksum += a[i];
    }
}

/* Test 2: Mix of dependence types (flow, anti, output) */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n, int *checksum) {
    int t = 0;
    
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: a[i] written after potential read */
        c[i] = c[i - 1] * 2;    /* Flow: c[i-1] -> c[i], Output: c[i] written */
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n, int *checksum) {
    /* No restrict qualifier - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N], int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
static void test_volatile_access(int *checksum) {
    volatile int v[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        v[i] = i;
    }
    
    /* Chain of volatile dependences */
    for (int i = 1; i < SIZE; i++) {
        v[i] = v[i - 1] + v[i] * 3;
    }
    
    for (int i = 0; i < SIZE; i++) {
        *checksum += v[i];
    }
}

/* Test 6: Different data types for varied data_type field */
__attribute__((always_inline))
static inline void test_mixed_data_types(float *fa, double *da, int n, int *checksum) {
    /* Float array with flow dependence */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;
    }
    
    /* Double array with anti-dependence */
    double temp = 0.0;
    for (int i = 0; i < n; i++) {
        temp = da[i];      /* Read da[i] */
        da[i] = temp * 2.0; /* Write da[i] - anti-dependence */
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += (int)fa[i] + (int)da[i];
    }
}

/* Test 7: Complex index calculations with potential WAR/WAW */
static void test_complex_indices(int *a, int *b, int n, int *checksum) {
    int idx1, idx2;
    
    for (int i = 0; i < n; i++) {
        idx1 = (i * 3) % n;
        idx2 = (i * 5) % n;
        
        /* Potential WAR: a[idx1] read, then a[idx2] written (may overlap) */
        /* Potential WAW: a[idx2] written multiple times if idx2 repeats */
        a[idx2] = b[i] + a[idx1];
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += a[i];
    }
}

int main() {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(N * sizeof(float));
    double *da = (double*)malloc(N * sizeof(double));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
    }
    
    for (int i = 0; i < N; i++) {
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to prevent dead code elimination */
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, (int*)&checksum);
    test_mixed_dependences(a, b, c, SIZE, (int*)&checksum);
    test_pointer_aliasing(a, b, SIZE, (int*)&checksum);
    test_nested_loops(arr, (int*)&checksum);
    test_volatile_access((int*)&checksum);
    test_mixed_data_types(fa, da, N, (int*)&checksum);
    test_complex_indices(a, b, SIZE, (int*)&checksum);
    
    /* Use the checksum to prevent optimization */
    printf("Result checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    
    return 0;
}
