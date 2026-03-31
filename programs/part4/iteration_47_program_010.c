#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
static void __attribute__((noinline,noipa)) 
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

static void __attribute__((noinline,noipa))
test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within same iteration */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int t = arr[i] * 2;
        
        /* WAR: Overwrite arr[i] after reading it */
        arr[i] = t + i;
        
        /* WAW: Conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] * 3;  /* Another write to arr[i] */
        }
    }
}

static void __attribute__((noinline,noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 2 + i;
    }
}

static void __attribute__((noinline,noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Multiple interleaved dependencies */
    for (int i = 3; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = x[i-1];
        int t2 = y[i-2];
        int t3 = z[i-3];
        
        /* WAR: Write after reading x[i] */
        int old_x = x[i];
        x[i] = t1 + t2;
        
        /* WAW: Multiple writes to y[i] */
        y[i] = t2 * 2;
        if (i % 4 == 0) {
            y[i] = t3 + 5;  /* Second write to y[i] */
        }
        
        /* Loop-carried with varying distances */
        z[i] = z[i-1] + z[i-2] + old_x;
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant for optimizer */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  /* Ensure minimum size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    /* Call test functions to create DDG edges */
    test_raw_dep(a, b, n);
    test_mixed_deps(c, n);
    test_distance_2(d, n);
    test_complex_deps(a, b, c, n);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
