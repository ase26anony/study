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
    // Mixed dependencies within same loop iteration
    for (int i = 0; i < n; i++) {
        // RAW: read arr[i]
        int temp = arr[i];
        
        // WAR: write to arr[i] after reading it
        arr[i] = temp * 2 + i;
        
        // WAW: potential second write to arr[i]
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  // Second write to same location
        }
    }
}

__attribute__((noinline, noipa))
static void test_distance_2(int *a, int n) {
    // Loop-carried RAW dependency with distance 2
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

// Additional test with volatile to prevent optimization
__attribute__((noinline, noipa))
static void test_volatile_dep(volatile int *a, int *b, int n) {
    // Volatile creates memory barrier, preserving dependencies
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

// Complex loop with multiple dependencies
__attribute__((noinline, noipa))
static void test_complex_deps(int *a, int *b, int *c, int n) {
    for (int i = 2; i < n; i++) {
        // Multiple RAW dependencies
        int t1 = a[i-1];
        int t2 = b[i-2];
        
        // WAR: read then write to a[i]
        a[i] = t1 + t2;
        
        // WAW: conditional second write
        if (c[i] > 0) {
            a[i] = a[i] * 2;
        }
        
        // Cross-iteration dependency with distance 1
        b[i] = a[i-1] + c[i];
    }
}

int main(int argc, char *argv[]) {
    // Use argc to make size non-constant (prevents constant propagation)
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  // Ensure minimum size
    
    // Seed random number generator
    srand(time(NULL));
    
    // Allocate dynamic arrays to prevent static analysis
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *arr3 = (int*)malloc(size * sizeof(int));
    int *arr4 = (int*)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with random values to prevent constant propagation
    for (int i = 0; i < size; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
    }
    
    // Call test functions to create various DDG edges
    test_raw_dep(arr1, arr2, size);
    test_mixed_deps(arr3, size);
    test_distance_2(arr4, size);
    
    // Test with volatile
    volatile int *vol_arr = (volatile int*)malloc(size * sizeof(int));
    if (vol_arr) {
        for (int i = 0; i < size; i++) {
            vol_arr[i] = rand() % 100;
        }
        test_volatile_dep(vol_arr, arr2, size);
        free((void*)vol_arr);
    }
    
    // Complex dependencies test
    test_complex_deps(arr1, arr2, arr3, size);
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    // Cleanup
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
