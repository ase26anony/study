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
        int temp = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: Conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
    }
}

static void __attribute__((noinline,noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

static void __attribute__((noinline,noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    /* Complex pattern with multiple dependency types */
    for (int i = 1; i < n; i++) {
        /* RAW on x with distance 1 */
        int t1 = x[i-1];
        
        /* WAR: Read y[i] then write to it */
        int t2 = y[i];
        y[i] = t1 + t2;
        
        /* WAW: Two writes to z[i] */
        z[i] = t1 * 2;
        if (t2 > 0) {
            z[i] = z[i] + t2;  /* Second write to z[i] */
        }
        
        /* RAW on x with distance 0 (within iteration) */
        x[i] = z[i] + i;
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant for optimizer */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    int *e = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with input-dependent values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        e[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(d, size);
    test_complex_deps(e, a, b, size);  /* Reuse arrays for aliasing */
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + e[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e);
    
    return 0;
}
