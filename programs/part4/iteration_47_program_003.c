#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
__attribute__((noinline, noipa))
static void test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

__attribute__((noinline, noipa))
static void test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within same loop iteration */
    for (int i = 0; i < n; i++) {
        /* RAW dependency: read arr[i] */
        int temp = arr[i];
        
        /* WAR dependency: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW dependency: conditional second write to arr[i] */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to same location */
        }
    }
}

__attribute__((noinline, noipa))
static void test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

__attribute__((noinline, noipa))
static void test_complex_deps(int *x, int *y, int n) {
    /* Complex dependencies with multiple arrays */
    for (int i = 1; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = x[i-1];
        int t2 = y[i];
        
        /* WAR: read x[i] then write to it */
        int t3 = x[i];
        x[i] = t1 + t2;
        
        /* WAW: two writes to y[i] */
        y[i] = t3 * 2;
        if (i % 4 == 0) {
            y[i] = y[i] + 50;  /* Second write */
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *arr3 = (int*)malloc(size * sizeof(int));
    int *arr4 = (int*)malloc(size * sizeof(int));
    int *arr5 = (int*)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
        arr5[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(arr1, arr2, size);
    test_mixed_deps(arr3, size);
    test_distance_2(arr4, size);
    test_complex_deps(arr5, arr1, size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i] + arr5[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
