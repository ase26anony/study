#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and IPA to preserve dependencies */
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
        /* RAW: read arr[i] */
        int temp = arr[i];
        
        /* WAR: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: potential second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* WAW dependency */
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
    /* Multiple interleaved dependencies */
    for (int i = 1; i < n; i++) {
        /* RAW on x with distance 1 */
        int t1 = x[i-1];
        
        /* RAW on y within same iteration */
        int t2 = y[i];
        
        /* WAR: write to x[i] after reading x[i-1] */
        x[i] = t1 + t2;
        
        /* Another RAW on z */
        z[i] = z[i] + x[i];
        
        /* WAW on y */
        if (i % 4 == 0) {
            y[i] = z[i] * 2;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  /* Ensure non-trivial size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-constant values */
    srand(time(NULL));
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
    test_complex_deps(a, b, c, n); /* Complex dependency pattern */
    
    /* Compute checksum to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %lld\n", sum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
