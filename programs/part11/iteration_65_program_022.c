#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 64

/* Force inlining to create larger basic blocks */
__attribute__((always_inline))
static inline int compute_checksum(int a, int b) {
    return a ^ (b * 31);
}

/* Test 1: Loop-carried flow dependence with distance > 0 */
void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    volatile int *va = (volatile int *)a;
    volatile int *vb = (volatile int *)b;
    
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        va[i + 2] = va[i] * vb[i] + 1;
    }
    
    /* Accumulate results */
    for (int i = 0; i < n; i++) {
        *checksum = compute_checksum(*checksum, va[i]);
    }
}

/* Test 2: Mix of dependence types (RAW, WAR, WAW) */
void test_mixed_dependences(int *a, int *b, int *c, int n, int *checksum) {
    volatile int t;
    
    for (int i = 1; i < n; i++) {
        /* RAW (flow) dependence on a[i-1] */
        t = a[i - 1];
        
        /* WAR (anti) dependence on t */
        a[i] = t + b[i];
        
        /* WAW (output) dependence on c[i] */
        c[i] = c[i - 1] * 2;
        
        /* Another RAW on c[i-1] */
        c[i] += c[i - 1];
    }
    
    for (int i = 0; i < n; i++) {
        *checksum = compute_checksum(*checksum, a[i] + c[i]);
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *p, int *q, int n, int *checksum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        /* Potential flow dependence due to possible p self-overlap */
        p[i] = q[i] + p[(i * 7) % n] + p[(i * 3) % n];
    }
    
    for (int i = 0; i < n; i++) {
        *checksum = compute_checksum(*checksum, p[i]);
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
void test_nested_loops(int arr[M][N], int *checksum) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + i;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            *checksum = compute_checksum(*checksum, arr[i][j]);
        }
    }
}

/* Test 5: Volatile arrays forcing memory dependencies */
void test_volatile_dependence(int *checksum) {
    volatile int v1[100];
    volatile int v2[100];
    
    /* Strong flow dependence through volatile */
    for (int i = 1; i < 100; i++) {
        v1[i] = v1[i - 1] + v2[i] + i;
    }
    
    /* Anti-dependence with volatile */
    for (int i = 99; i > 0; i--) {
        int temp = v1[i];
        v1[i - 1] = temp * 2;
        v2[i] = v1[i] + 3;
    }
    
    for (int i = 0; i < 100; i++) {
        *checksum = compute_checksum(*checksum, v1[i] + v2[i]);
    }
}

/* Test 6: Different data types to affect data_type field */
void test_mixed_data_types(float *fa, double *da, int *ia, int n, int *checksum) {
    /* Mixed type dependencies */
    for (int i = 1; i < n; i++) {
        /* Flow dependence through different types */
        fa[i] = fa[i - 1] * 1.5f;
        da[i] = da[i - 1] + fa[i];
        ia[i] = (int)da[i] + ia[i - 1];
    }
    
    for (int i = 0; i < n; i++) {
        *checksum = compute_checksum(*checksum, ia[i] + (int)fa[i]);
    }
}

/* Test 7: Complex index calculations with modulo */
void test_complex_indices(int *arr, int n, int *checksum) {
    /* Complex addressing that's hard to analyze */
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 13) % n;
        int idx2 = (i * 17) % n;
        int idx3 = (i * 23) % n;
        
        /* Multiple potential dependencies */
        arr[idx1] = arr[idx2] + arr[idx3] + i;
    }
    
    for (int i = 0; i < n; i++) {
        *checksum = compute_checksum(*checksum, arr[i]);
    }
}

int main() {
    int checksum = 0;
    
    /* Initialize with deterministic but non-trivial values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    int *p = (int *)malloc(N * sizeof(int));
    int *q = (int *)malloc(N * sizeof(int));
    int *arr_flat = (int *)malloc(N * sizeof(int));
    float *fa = (float *)malloc(N * sizeof(float));
    double *da = (double *)malloc(N * sizeof(double));
    int *ia = (int *)malloc(N * sizeof(int));
    int arr_2d[M][N];
    
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        arr_flat[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 3.0f;
        da[i] = (double)(rand() % 100) / 7.0;
        ia[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr_2d[i][j] = rand() % 100;
        }
    }
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, N, &checksum);
    test_mixed_dependences(a, b, c, N, &checksum);
    test_pointer_aliasing(p, q, N, &checksum);
    test_nested_loops(arr_2d, &checksum);
    test_volatile_dependence(&checksum);
    test_mixed_data_types(fa, da, ia, N, &checksum);
    test_complex_indices(arr_flat, N, &checksum);
    
    /* Final computation to ensure all results are used */
    for (int i = 0; i < N; i++) {
        checksum = compute_checksum(checksum, 
            a[i] + b[i] + c[i] + p[i] + q[i] + arr_flat[i] + ia[i]);
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(p); free(q);
    free(arr_flat); free(fa); free(da); free(ia);
    
    return 0;
}
