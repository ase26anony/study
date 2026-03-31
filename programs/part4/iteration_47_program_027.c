#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
__attribute__((noinline, noipa))
static void test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

__attribute__((noinline, noipa))
static void test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within the same loop */
    for (int i = 0; i < n; i++) {
        /* RAW dependency: read arr[i] */
        int temp = arr[i];
        
        /* WAR dependency: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW dependency: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
    }
}

__attribute__((noinline, noipa))
static void test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

__attribute__((noinline, noipa))
static void test_complex_deps(int *x, int *y, int *z, int n) {
    /* Complex pattern with multiple dependency types and distances */
    for (int i = 3; i < n; i++) {
        /* Multiple RAW dependencies with different distances */
        int t1 = x[i-1];  /* distance 1 */
        int t2 = y[i-2];  /* distance 2 */
        int t3 = z[i-3];  /* distance 3 */
        
        /* WAR: read then write to same array */
        x[i] = t1 + t2;
        
        /* WAW: potential second write to x[i] */
        if (t3 > 0) {
            x[i] = x[i] * 2;
        }
        
        /* Additional RAW on y */
        y[i] = y[i-1] + 1;
    }
}

/* Use volatile to prevent certain optimizations */
volatile int global_seed = 42;

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  /* Ensure non-trivial size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with input-dependent values to prevent optimization */
    srand(time(NULL) ^ global_seed);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, n);      /* Creates RAW edges with distance 1 */
    test_mixed_deps(c, n);      /* Creates RAW, WAR, WAW edges */
    test_distance_2(d, n);      /* Creates RAW edges with distance 2 */
    test_complex_deps(a, b, c, n); /* Complex pattern */
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
