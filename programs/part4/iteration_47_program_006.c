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
    /* Mixed dependencies within a single loop iteration */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int temp = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: Potential second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to same location */
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
        
        /* WAR: Read then write to same array */
        x[i] = t1 + t2;
        
        /* Another RAW with different distance */
        y[i] = x[i-2] + t3;
        
        /* WAW: Two writes to z[i] in same iteration */
        z[i] = x[i] + y[i];
        if (i % 4 == 0) {
            z[i] = z[i] * 2;  /* Second write */
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make bounds non-constant for the compiler */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  /* Ensure reasonable size */
    
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
    test_raw_dep(a, b, n);      /* RAW edges with distance 1 */
    test_mixed_deps(c, n);      /* RAW, WAR, WAW edges */
    test_distance_2(d, n);      /* RAW edges with distance 2 */
    test_complex_deps(a, b, e, n); /* Complex pattern */
    
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
