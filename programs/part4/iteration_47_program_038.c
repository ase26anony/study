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
    /* Mixed dependencies within loop */
    for (int i = 0; i < n; i++) {
        /* RAW: read arr[i] */
        int t = arr[i];
        
        /* WAR: write to arr[i] after reading it */
        arr[i] = t * 2 + i;
        
        /* WAW: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
        
        /* Use t to prevent dead code elimination */
        arr[i] += (t & 1);
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3 + i;
    }
}

static void __attribute__((noinline, noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Complex pattern with multiple dependency types */
    for (int i = 1; i < n; i++) {
        /* RAW on x with distance 1 */
        int tmp = x[i-1];
        
        /* WAR: read y[i] then write to it */
        int y_val = y[i];
        y[i] = tmp + i;
        
        /* WAW: two writes to z[i] */
        z[i] = y_val * 2;
        if (tmp > 0) {
            z[i] = z[i] + tmp;  /* Second write to z[i] */
        }
        
        /* RAW with distance 1 on x */
        x[i] = z[i] - y_val;
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
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
