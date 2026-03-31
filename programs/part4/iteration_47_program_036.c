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
    volatile int sink; /* Prevent dead code elimination */
    
    for (int i = 0; i < n; i++) {
        /* RAW dependency: read arr[i] */
        int temp = arr[i];
        
        /* WAR dependency: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW dependency: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100; /* Second write to arr[i] */
        }
        
        sink = temp; /* Use temp to prevent elimination */
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
        
        /* RAW on y with distance 1 */
        y[i] = y[i-1] + t1;
        
        /* WAR: read z[i] then write to it */
        int t2 = z[i];
        z[i] = t2 * 2 + i;
        
        /* WAW: two writes to x[i] */
        x[i] = t1 + i;
        if (i % 4 == 0) {
            x[i] = x[i] - 50; /* Second write */
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant for the optimizer */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
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
    
    /* Initialize with input-dependent values to prevent optimization */
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
    test_complex_deps(a, e, c, size);
    
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
