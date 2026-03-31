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
    /* Loop-carried RAW with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

static void __attribute__((noinline,noipa))
test_complex_deps(int *x, int *y, int n) {
    /* Complex pattern with multiple dependency types */
    for (int i = 1; i < n; i++) {
        /* RAW on x with distance 1 */
        int t1 = x[i-1];
        
        /* RAW on y within same iteration */
        int t2 = y[i];
        
        /* WAR: Write to x[i] after reading x[i-1] */
        x[i] = t1 + t2;
        
        /* Another RAW on the newly written value */
        y[i] = x[i] * 2;
        
        /* WAW: Conditional write to y[i] */
        if (i % 4 == 0) {
            y[i] = y[i] + 50;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc to make bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  /* Ensure non-tiny size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *arr = (int*)malloc(n * sizeof(int));
    int *x = (int*)malloc(n * sizeof(int));
    int *y = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !arr || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values to prevent optimization */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        arr[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call test functions to create DDG edges */
    test_raw_dep(a, b, n);
    test_mixed_deps(arr, n);
    test_distance_2(a, n);
    test_complex_deps(x, y, n);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + arr[i] + x[i] + y[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(arr);
    free(x);
    free(y);
    
    return 0;
}
