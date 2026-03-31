/* test_ddg_coverage.c
 * 
 * This program creates various loop patterns to trigger DDG edge creation
 * with different characteristics (flow, anti, output dependencies,
 * different distances, data types, and aliasing patterns).
 * 
 * Compile with: gcc -O3 -fmodulo-sched -funroll-loops -floop-nest-optimize test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define M 256
#define K 128

/* Volatile arrays to force memory dependencies */
volatile int volatile_array[N];
volatile float volatile_float_array[N];

/* Test 1: Simple loop-carried flow dependence with distance 2 */
__attribute__((always_inline))
static inline int test_flow_dependence_distance_2(int *a, int *b, int n) {
    int sum = 0;
    /* Flow dependence: a[i] read, a[i+2] written, distance = 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
        sum += a[i + 2];
    }
    return sum;
}

/* Test 2: Mixed dependence types (flow, anti, output) */
__attribute__((always_inline))
static inline int test_mixed_dependences(int *a, int *b, int *c, int n) {
    int t = 0;
    int sum = 0;
    
    /* Contains multiple dependence types:
     * - Flow: a[i-1] -> t -> a[i] (distance 1)
     * - Anti: read a[i], then write a[i] (WAR within iteration)
     * - Output: a[i] written multiple times (WAW)
     */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Flow dependence from previous iteration */
        a[i] = t + b[i];            /* Anti-dependence on a[i] from above read? */
        c[i] = c[i - 1] * 2;        /* Flow dependence on c[i-1], distance 1 */
        a[i] = a[i] + c[i];         /* Output dependence on a[i] (WAW) */
        sum += a[i] + c[i];
    }
    return sum;
}

/* Test 3: Pointer aliasing with potential self-overlap */
static int test_pointer_aliasing(int *p, int *q, int n) {
    int sum = 0;
    /* Complex indexing creates potential flow dependences
     * that alias analysis can't fully resolve */
    for (int i = 0; i < n; i++) {
        int idx = (i * 7) % n;
        p[i] = q[i] + p[idx] + i;  /* Possible flow if idx == i-k for some k */
        sum += p[i];
    }
    return sum;
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline int test_nested_loops(int arr[M][N]) {
    int sum = 0;
    /* Inner loop has flow dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + i + j;
            sum += arr[i][j];
        }
    }
    return sum;
}

/* Test 5: Volatile memory accesses forcing dependencies */
static int test_volatile_dependencies(void) {
    int sum = 0;
    /* Volatile ensures memory accesses aren't reordered/eliminated */
    for (int i = 1; i < N; i++) {
        volatile_array[i] = volatile_array[i - 1] + i * 2;
        sum += volatile_array[i];
    }
    return sum;
}

/* Test 6: Different data types (float/double) */
__attribute__((always_inline))
static inline float test_float_dependencies(float *fa, float *fb, int n) {
    float sum = 0.0f;
    /* Float array with flow dependence distance 3 */
    for (int i = 0; i < n - 3; i++) {
        fa[i + 3] = fa[i] * fb[i] + 0.5f;
        sum += fa[i + 3];
    }
    return sum;
}

/* Test 7: Complex index calculations with multiple arrays */
static int test_complex_indices(int *a, int *b, int *c, int n) {
    int sum = 0;
    /* Multiple interleaved dependencies with non-linear indices */
    for (int i = 2; i < n - 2; i++) {
        int idx1 = (i * 3) % n;
        int idx2 = (i * 5) % n;
        
        /* Create web of dependencies */
        a[i] = b[idx1] + c[idx2];
        b[i] = a[i - 2] * 3;
        c[i] = b[i - 1] + a[i];
        
        sum += a[i] + b[i] + c[i];
    }
    return sum;
}

/* Test 8: Restrict vs non-restrict pointer patterns */
static int test_restrict_patterns(int * restrict r1, int * restrict r2, 
                                  int *nr1, int *nr2, int n) {
    int sum = 0;
    /* Loop with restrict pointers - may have fewer assumed dependencies */
    for (int i = 1; i < n; i++) {
        r1[i] = r1[i - 1] + r2[i];
        sum += r1[i];
    }
    
    /* Loop without restrict - more conservative dependence analysis */
    for (int i = 1; i < n; i++) {
        nr1[i] = nr1[i - 1] + nr2[i];
        sum += nr1[i];
    }
    
    return sum;
}

/* Initialize arrays with pseudo-random but deterministic values */
static void initialize_arrays(int *a, int *b, int *c, float *fa, float *fb, 
                             int arr[M][N], int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 17) % 100;
        b[i] = (i * 23) % 100;
        c[i] = (i * 29) % 100;
        fa[i] = (float)(i % 50) * 0.7f;
        fb[i] = (float)(i % 30) * 1.3f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = (i * 31 + j * 37) % 200;
        }
    }
    
    /* Initialize volatile arrays */
    for (int i = 0; i < N; i++) {
        volatile_array[i] = i * 3;
        volatile_float_array[i] = (float)i * 0.5f;
    }
}

int main(void) {
    /* Allocate and align arrays for better performance */
    int *a = (int*)aligned_alloc(64, N * sizeof(int));
    int *b = (int*)aligned_alloc(64, N * sizeof(int));
    int *c = (int*)aligned_alloc(64, N * sizeof(int));
    int *r1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *r2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *nr1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *nr2 = (int*)aligned_alloc(64, N * sizeof(int));
    float *fa = (float*)aligned_alloc(64, N * sizeof(float));
    float *fb = (float*)aligned_alloc(64, N * sizeof(float));
    
    /* 2D array for nested loop test */
    int (*arr)[N] = (int(*)[N])aligned_alloc(64, M * N * sizeof(int));
    
    if (!a || !b || !c || !r1 || !r2 || !nr1 || !nr2 || !fa || !fb || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    initialize_arrays(a, b, c, fa, fb, arr, N);
    memcpy(r1, a, N * sizeof(int));
    memcpy(r2, b, N * sizeof(int));
    memcpy(nr1, a, N * sizeof(int));
    memcpy(nr2, b, N * sizeof(int));
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    checksum += test_flow_dependence_distance_2(a, b, N);
    checksum += test_mixed_dependences(a, b, c, N);
    checksum += test_pointer_aliasing(a, b, N);
    checksum += test_nested_loops(arr);
    checksum += test_volatile_dependencies();
    checksum += (int)test_float_dependencies(fa, fb, N);
    checksum += test_complex_indices(a, b, c, N);
    checksum += test_restrict_patterns(r1, r2, nr1, nr2, N);
    
    /* Additional loop to increase DDG complexity with unrolling */
    #pragma GCC unroll 4
    for (int i = 0; i < 10; i++) {
        checksum += test_flow_dependence_distance_2(a, b, N / 2);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(r1); free(r2); free(nr1); free(nr2);
    free(fa); free(fb);
    free(arr);
    
    return 0;
}
