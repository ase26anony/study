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
    /* Mixed dependencies within same iteration */
    for (int i = 0; i < n; i++) {
        /* RAW: read arr[i] */
        int t = arr[i];
        
        /* WAR: write to arr[i] after reading it */
        arr[i] = t * 2 + i;
        
        /* WAW: potential second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* WAW dependency */
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
    /* Complex pattern with multiple dependencies */
    for (int i = 3; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = x[i-1];
        int t2 = y[i-2];
        int t3 = z[i-3];
        
        /* WAR: read then write to same location */
        int old_x = x[i];
        x[i] = t1 + t2;
        
        /* WAW: two writes to y[i] */
        y[i] = t3 * 2;
        if (i % 4 == 0) {
            y[i] = y[i] + 50;  /* WAW */
        }
        
        /* Loop-carried with varying distances */
        z[i] = z[i-1] + z[i-2];
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
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
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %d\n", sum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
