#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define M 128

/* Force inlining to create larger basic blocks */
__attribute__((always_inline))
static inline int compute_checksum(int a, int b) {
    return a ^ (b * 31);
}

/* Test 1: Loop-carried flow dependence with distance > 0 */
int test1_flow_dependence_distance(int *a, int *b, int n) {
    int sum = 0;
    /* Flow dependence: a[i] written, read in next iteration with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
        sum += a[i + 2];
    }
    return sum;
}

/* Test 2: Multiple dependence types in single loop */
int test2_mixed_dependences(int *a, int *b, int *c, int n) {
    int t = 0;
    int sum = 0;
    
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        /* Flow (RAW): read a[i-1] after previous iteration's write */
        t = a[i - 1] + i;
        
        /* Anti (WAR): read b[i] before writing to a[i] */
        a[i] = t + b[i];
        
        /* Output (WAW): c[i] written, also written in next iteration */
        c[i] = c[i - 1] * 2 + a[i];
        
        sum += c[i];
    }
    return sum;
}

/* Test 3: Indirect addressing with potential aliasing */
int test3_indirect_addressing(int *p, int *q, int n) {
    int sum = 0;
    /* Complex indexing creates potential flow dependences */
    for (int i = 0; i < n; i++) {
        int idx = (i * 7) % n;
        p[i] = q[i] + p[idx] + i;
        sum += p[i];
    }
    return sum;
}

/* Test 4: Without restrict - forces conservative alias analysis */
int test4_no_restrict(int *p, int *q, int n) {
    int sum = 0;
    for (int i = 1; i < n; i++) {
        p[i] = q[i - 1] + p[i];  /* Potential flow if p == q */
        sum += p[i];
    }
    return sum;
}

/* Test 5: With restrict - allows more aggressive optimization */
int test5_with_restrict(int *__restrict p, int *__restrict q, int n) {
    int sum = 0;
    for (int i = 1; i < n; i++) {
        p[i] = q[i - 1] + p[i];  /* No aliasing possible */
        sum += p[i];
    }
    return sum;
}

/* Test 6: Nested loops with inner loop carried dependence */
int test6_nested_loops(int arr[M][N]) {
    int sum = 0;
    /* Inner loop has distance-2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + i + j;
            sum += arr[i][j];
        }
    }
    return sum;
}

/* Test 7: Volatile memory accesses - force memory dependences */
int test7_volatile_access(void) {
    volatile int v[100];
    int sum = 0;
    
    /* Strong memory dependence chain */
    v[0] = 1;
    for (int i = 1; i < 100; i++) {
        v[i] = v[i - 1] + i * 3;
        sum += v[i];
    }
    return sum;
}

/* Test 8: Different data types for varied data_type field */
int test8_mixed_data_types(float *fa, double *da, int *ia, int n) {
    int sum = 0;
    /* Mix of data types in dependence chain */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f + i;
        da[i] = da[i - 1] + fa[i];
        ia[i] = (int)da[i] + ia[i - 1];
        sum += ia[i];
    }
    return sum;
}

/* Test 9: Complex loop with multiple distances */
int test9_multiple_distances(int *a, int *b, int *c, int n) {
    int sum = 0;
    /* Multiple interleaved dependences with different distances */
    for (int i = 3; i < n; i++) {
        /* Distance 1 flow */
        a[i] = b[i - 1] + 1;
        
        /* Distance 2 flow */
        b[i] = c[i - 2] * 2;
        
        /* Distance 3 anti */
        c[i] = a[i - 3] + b[i];
        
        sum += a[i] + b[i] + c[i];
    }
    return sum;
}

/* Test 10: Pointer chasing creating long dependence chain */
int test10_pointer_chasing(int *data, int *next, int n) {
    int sum = 0;
    int idx = 0;
    for (int i = 0; i < n; i++) {
        idx = next[idx];
        data[i] = data[idx] + i;
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Initialize with deterministic but non-trivial pattern */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *p = (int*)malloc(N * sizeof(int));
    int *q = (int*)malloc(N * sizeof(int));
    float *fa = (float*)malloc(N * sizeof(float));
    double *da = (double*)malloc(N * sizeof(double));
    int *ia = (int*)malloc(N * sizeof(int));
    int arr[M][N];
    int *data = (int*)malloc(N * sizeof(int));
    int *next = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 5.0;
        ia[i] = rand() % 100;
        data[i] = rand() % 100;
        next[i] = (i + 1) % N;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    checksum = compute_checksum(checksum, test1_flow_dependence_distance(a, b, N));
    checksum = compute_checksum(checksum, test2_mixed_dependences(a, b, c, N));
    checksum = compute_checksum(checksum, test3_indirect_addressing(p, q, N));
    checksum = compute_checksum(checksum, test4_no_restrict(p, q, N));
    checksum = compute_checksum(checksum, test5_with_restrict(p, q, N));
    checksum = compute_checksum(checksum, test6_nested_loops(arr));
    checksum = compute_checksum(checksum, test7_volatile_access());
    checksum = compute_checksum(checksum, test8_mixed_data_types(fa, da, ia, N));
    checksum = compute_checksum(checksum, test9_multiple_distances(a, b, c, N));
    checksum = compute_checksum(checksum, test10_pointer_chasing(data, next, N));
    
    /* Print result to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(p); free(q);
    free(fa); free(da); free(ia);
    free(data); free(next);
    
    return 0;
}
