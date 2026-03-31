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
        int temp = arr[i] * 2;
        
        /* WAR: write to arr[i] after reading it */
        arr[i] = temp + i;
        
        /* WAW: potential second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] * 3;  /* Another write to same location */
        }
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 2 + i;
    }
}

static void __attribute__((noinline, noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Multiple overlapping dependencies */
    for (int i = 3; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = x[i-1];
        int t2 = y[i-2];
        int t3 = z[i-3];
        
        /* WAR: write after reads */
        x[i] = t1 + i;
        y[i] = t2 * 2;
        
        /* WAW: multiple writes to z[i] */
        z[i] = t3 + t1;
        if (i % 4 == 0) {
            z[i] = z[i] + 100;  /* Second write */
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc to make bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  /* Ensure non-trivial size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        e[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, n);           /* RAW edges with distance 1 */
    test_mixed_deps(c, n);           /* Mixed RAW/WAR/WAW edges */
    test_distance_2(d, n);           /* RAW edges with distance 2 */
    test_complex_deps(a, b, e, n);   /* Complex overlapping deps */
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + e[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e);
    
    return 0;
}
