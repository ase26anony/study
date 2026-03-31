#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural analysis
__attribute__((noinline, noipa))
static void test_raw_dep(int *a, int *b, int n) {
    // Loop-carried RAW dependency with distance 1
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

__attribute__((noinline, noipa))
static void test_mixed_deps(int *arr, int n) {
    // Mixed dependencies within a single loop
    for (int i = 0; i < n; i++) {
        // RAW dependency: read arr[i]
        int t = arr[i] * 2;
        
        // WAR dependency: write to arr[i] after reading it
        arr[i] = t + i;
        
        // WAW dependency: conditional second write to same location
        if (i % 3 == 0) {
            arr[i] = arr[i] * 3;  // Second write to arr[i]
        }
    }
}

__attribute__((noinline, noipa))
static void test_distance_2(int *a, int n) {
    // Loop-carried RAW dependency with distance 2
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 2 + i;
    }
}

__attribute__((noinline, noipa))
static void test_complex_deps(int *x, int *y, int *z, int n) {
    // Complex pattern with multiple dependency types
    for (int i = 1; i < n; i++) {
        // Multiple RAW dependencies
        int t1 = x[i-1];
        int t2 = y[i];
        
        // WAR: read z[i], then write to it
        int t3 = z[i];
        
        // Write with potential WAW
        x[i] = t1 + t2;
        
        // WAR: overwrite z[i] after reading it
        z[i] = t3 * 2;
        
        // Conditional WAW on x[i]
        if (i % 4 == 0) {
            x[i] = x[i] + 100;
        }
    }
}

int main(int argc, char *argv[]) {
    // Use argc to make loop bounds non-constant
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  // Ensure reasonable size
    
    // Allocate dynamic arrays to prevent constant propagation
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random values to prevent optimization
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        e[i] = rand() % 100;
    }
    
    // Call test functions to create various DDG edges
    test_raw_dep(a, b, n);           // Simple RAW edges
    test_mixed_deps(c, n);           // Mixed RAW/WAR/WAW edges
    test_distance_2(d, n);           // Distance-2 edges
    test_complex_deps(a, b, e, n);   // Complex pattern
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + e[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    return 0;
}
