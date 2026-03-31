#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
static void __attribute__((noinline, noipa))
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

static void __attribute__((noinline, noipa))
test_mixed_deps(int *arr, int n) {
    /* Contains RAW, WAR, and WAW dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW: read arr[i] */
        int temp = arr[i];
        
        /* WAR: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

static void __attribute__((noinline, noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Multiple interleaved dependencies */
    for (int i = 1; i < n; i++) {
        /* RAW with distance 1 */
        int t1 = x[i-1] + y[i];
        
        /* WAR: read then write to y[i] */
        int t2 = y[i];
        y[i] = t1 + t2;
        
        /* WAW: two writes to z[i] */
        z[i] = t1 * 2;
        if (t2 > 0) {
            z[i] = z[i] + t2;  /* Second write to z[i] */
        }
        
        /* RAW with variable distance */
        if (i >= 3) {
            x[i] = x[i-3] + z[i];
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure non-tiny size */
    
    /* Dynamic allocation to avoid stack-based optimizations */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with input-dependent values to prevent constant folding */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    /* Call test functions to create DDG edges */
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(d, size);
    test_complex_deps(a, b, c, size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
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
