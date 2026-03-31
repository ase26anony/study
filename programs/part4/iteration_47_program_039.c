#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural analysis
static void __attribute__((noinline, noipa)) 
test_raw_dep(int *a, int *b, int n) {
    // Loop-carried RAW dependency with distance 1
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

static void __attribute__((noinline, noipa))
test_mixed_deps(int *arr, int n) {
    // Mixed dependencies within same loop iteration
    for (int i = 0; i < n; i++) {
        // RAW: Read arr[i]
        int temp = arr[i];
        
        // WAR: Write to arr[i] after reading it
        arr[i] = temp * 2 + i;
        
        // WAW: Conditional second write to same location
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  // Second write to arr[i]
        }
    }
}

static void __attribute__((noinline, noipa))
test_distance_2(int *a, int n) {
    // Loop-carried RAW dependency with distance 2
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

static void __attribute__((noinline, noipa))
test_complex_deps(int *x, int *y, int *z, int n) {
    // Complex pattern with multiple dependency types
    for (int i = 1; i < n; i++) {
        // RAW: y depends on previous x
        y[i] = x[i-1] + z[i];
        
        // WAR: x is read then written
        int tmp = x[i];
        x[i] = y[i] * 2;
        
        // Potential WAW through pointer aliasing
        if (tmp > 0) {
            x[i] = x[i] - tmp;
        }
        
        // Another RAW with different distance
        if (i >= 3) {
            z[i] = z[i-3] + 1;
        }
    }
}

int main(int argc, char **argv) {
    // Use non-constant size to prevent loop unrolling
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Dynamic allocation prevents constant propagation
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    // Initialize with non-constant values
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    // Call test functions to create various DDG edges
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(d, size);
    test_complex_deps(a, b, c, size);
    
    // Compute checksum to prevent dead code elimination
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
