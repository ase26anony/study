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
    /* Mixed dependencies within a single iteration */
    for (int i = 0; i < n; i++) {
        /* RAW dependency: read arr[i] */
        int temp = arr[i];
        
        /* WAR dependency: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW dependency: conditional second write to same location */
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
        
        /* RAW on y with distance 1 */
        int t2 = y[i-1];
        
        /* WAR: write to x[i] after reading x[i-1] */
        x[i] = t1 + t2;
        
        /* WAW: potential second write to x[i] */
        if (z[i] > 0) {
            x[i] = x[i] * 2;
        }
        
        /* RAW on newly written x[i] */
        y[i] = x[i] + z[i];
    }
}

/* Use volatile to prevent certain optimizations */
volatile int global_seed = 42;

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    /* Dynamic allocation prevents constant propagation */
    int *array1 = (int*)malloc(size * sizeof(int));
    int *array2 = (int*)malloc(size * sizeof(int));
    int *array3 = (int*)malloc(size * sizeof(int));
    int *array4 = (int*)malloc(size * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    srand(time(NULL) ^ global_seed);
    for (int i = 0; i < size; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        array4[i] = rand() % 100;
    }
    
    /* Call test functions to create DDG edges */
    test_raw_dep(array1, array2, size);
    test_mixed_deps(array3, size);
    test_distance_2(array4, size);
    test_complex_deps(array1, array2, array3, size);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i];
        /* Add some computation to prevent loop optimizations */
        checksum = (checksum * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
