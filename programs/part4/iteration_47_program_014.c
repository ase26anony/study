#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural analysis
__attribute__((noinline, noipa))
static void test_raw_dep(int *a, int *b, int n) {
    // Loop-carried RAW dependency with distance 1
    for (int i = 1; i < n; i++) {
        a[i] = a[i - 1] + b[i];
    }
}

__attribute__((noinline, noipa))
static void test_mixed_deps(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        // RAW dependency: read arr[i]
        int temp = arr[i];
        
        // WAR dependency: write to arr[i] after reading it
        arr[i] = temp * 2 + i;
        
        // WAW dependency: conditional second write to same location
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  // Second write to arr[i]
        }
    }
}

__attribute__((noinline, noipa))
static void test_distance_2(int *a, int n) {
    // Loop-carried RAW dependency with distance 2
    for (int i = 2; i < n; i++) {
        a[i] = a[i - 2] * 3;
    }
}

__attribute__((noinline, noipa))
static void test_complex_deps(int *x, int *y, int *z, int n) {
    // Multiple interleaved dependencies
    for (int i = 1; i < n; i++) {
        // RAW: y depends on previous x
        y[i] = x[i - 1] + 5;
        
        // RAW: x depends on previous y (creating cycle)
        x[i] = y[i - 1] * 2;
        
        // WAW: multiple writes to z[i]
        z[i] = x[i] + y[i];
        if (i % 4 == 0) {
            z[i] = z[i] * 2;  // Second write
        }
    }
}

int main(int argc, char **argv) {
    // Use non-constant size to prevent loop unrolling
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;  // Ensure non-tiny size
    
    // Dynamic allocation to avoid constant propagation
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *d = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with input-dependent values to prevent constant folding
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    // Call test functions to create various DDG edges
    test_raw_dep(a, b, n);
    test_mixed_deps(c, n);
    test_distance_2(d, n);
    
    // Create arrays for complex dependency test
    int *x = (int *)malloc(n * sizeof(int));
    int *y = (int *)malloc(n * sizeof(int));
    int *z = (int *)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        x[i] = rand() % 50;
        y[i] = rand() % 50;
        z[i] = rand() % 50;
    }
    
    test_complex_deps(x, y, z, n);
    
    // Compute checksum to prevent dead code elimination
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i] + z[i];
    }
    
    printf("Checksum: %d\n", sum);
    
    // Free allocated memory
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
